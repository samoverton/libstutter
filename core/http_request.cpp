#include "http_request.h"
#include "http_connection.h"

#include <iostream>
#include <sstream>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>


using namespace std;

HttpRequest::HttpRequest(HttpConnection &connection)
	: m_connection(connection)
	, m_error(NOT_EXECUTED)
{}

HttpRequest::HttpRequest(const HttpRequest &request)
	: m_url(request.m_url)
	, m_headers(request.m_headers)
	, m_connection(request.m_connection)
	, m_error(request.m_error)
{
}

const string&
HttpRequest::url() const
{
	return m_url;
}

void
HttpRequest::add_header(string key, string val)
{
	m_headers.insert(make_pair(key, val));
}

void
HttpRequest::add_url_fragment(const char *at, size_t sz)
{
	m_url.append(at, sz);
}

void
HttpRequest::set_host(string host)
{
	m_host = host;
}

HttpRequest::Error
HttpRequest::send(HttpReply &reply)
{
	prepare() && connect() && send() && read_reply(reply);
	return m_error;
}

bool
HttpRequest::prepare()
{
	add_header("Host", m_host);

	stringstream ss;
	string crlf("\r\n");
	ss << "GET " << m_url << " HTTP/1.1" << crlf;

	map<string,string>::const_iterator hi;
	for(hi = m_headers.begin(); hi != m_headers.end(); hi++) {
		ss << hi->first << ": " << hi->second << crlf;
	}
	ss << crlf;

	m_data = ss.str();
	return true;
}

bool
HttpRequest::connect()
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) {
		m_error = SOCKET_ERROR;
		return false; // TODO: log
	}

	// set socket as non-blocking.
	int ret = fcntl(fd, F_SETFD, O_NONBLOCK);
	if (0 != ret) {
		m_error = SOCKET_ERROR;
		return false; // TODO: log
	}

	struct addrinfo *info = 0;
	ret = getaddrinfo(m_host.c_str(), "8080", 0, &info);
	if (ret < 0) {
		m_error = DNS_ERROR;
		return false; // TODO: log
	}

	bool success = true;
	struct addrinfo *ai;
	for (ai = info; ai; ai = ai->ai_next) {
		struct sockaddr_in *sin = (struct sockaddr_in*)ai->ai_addr;
		int ret = ::connect(fd, (const struct sockaddr*)sin,
				sizeof(struct sockaddr_in));
		// cout << "Connect: ret=" << ret << endl;
		if (ret != 0) {
			success = false;
			m_error = CONNECTION_ERROR;
		}
		m_fd = fd;
		break;
	}
	freeaddrinfo(info);
	return success;
}

bool
HttpRequest::send()
{
	string::iterator i;
	for (i = m_data.begin(); i != m_data.end(); )
	{
		int sent = m_connection.safe_write(m_fd, &(*i), distance(i, m_data.end()));
		if (sent <= 0) {
			m_error = WRITE_ERROR;
			close(m_fd);
			return false;
		}
		i += sent;
	}
	return true;
}

void
_done(void *self)
{
	HttpRequest *req = reinterpret_cast<HttpRequest*>(self);
	req->m_error = HttpRequest::Error::SUCCESS;
}

bool
HttpRequest::read_reply(HttpReply &reply)
{
	HttpParser parser(HttpParser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));

	while(m_error != SUCCESS)
	{
		char buffer[1024];
		int recvd = m_connection.safe_read(m_fd, buffer, sizeof(buffer));
		if (recvd <= 0)
			m_connection.yield((int)HttpConnection::Need::HALT);

		bool success = parser.add(buffer, (size_t)recvd);
		if (!success)
			m_connection.yield((int)HttpConnection::Need::HALT);
	}

	close(m_fd);
	return true;
}
