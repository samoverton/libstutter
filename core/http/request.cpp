#include "request.h"
#include "connection.h"

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
using http::Request;
using http::Reply;

Request::Request(HttpConnection &connection)
	: m_connection(connection)
	, m_error(NOT_EXECUTED)
{}

Request::Request(const Request &request)
	: m_url(request.m_url)
	, m_headers(request.m_headers)
	, m_connection(request.m_connection)
	, m_error(request.m_error)
{
}

const string&
Request::url() const
{
	return m_url;
}

void
Request::add_header(string key, string val)
{
	m_headers.insert(make_pair(key, val));
}

void
Request::add_url_fragment(const char *at, size_t sz)
{
	m_url.append(at, sz);
}

void
Request::set_host(string host)
{
	m_host = host;
}

void
Request::reset()
{
	m_url.clear();
	m_host.clear();
	m_headers.clear();
	m_data.clear();
	m_error = NOT_EXECUTED;
}

Request::Error
Request::send(Reply &reply)
{
	prepare() && connect() && send() && read_reply(reply);
	return m_error;
}

void
Request::error(Error e)
{
	m_error = e;
}

bool
Request::prepare()
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
Request::connect()
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
		if (ret != 0) {
			success = false;
			m_error = CONNECTION_ERROR; // TODO: log
		}
		m_fd = fd;
		break;
	}
	freeaddrinfo(info);
	return success;
}

bool
Request::send()
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
	Request *req = reinterpret_cast<Request*>(self);
	req->error(http::Request::Error::SUCCESS);
}

bool
Request::read_reply(Reply &reply)
{
	reply.reset();
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
