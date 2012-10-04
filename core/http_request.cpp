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
{}

HttpRequest::HttpRequest(const HttpRequest &request)
	: m_url(request.m_url)
	, m_headers(request.m_headers)
	, m_connection(request.m_connection)
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

void
HttpRequest::send(HttpReply &reply)
{
	prepare();
	connect();
	send();
	read_reply(reply);
}

void
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
}

void
HttpRequest::connect()
{
	cout << "CONNECT" << endl;

	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == fd) {
		// TODO: log
		return;
	}

	// set socket as non-blocking.
	int ret = fcntl(fd, F_SETFD, O_NONBLOCK);
	if (0 != ret) {
		// TODO: log
		return;
	}

	struct addrinfo *info = 0;
	ret = getaddrinfo(m_host.c_str(), "8080", 0, &info);
	if (ret < 0)
		return; // TODO: log

	char *id = 0;
	struct addrinfo *ai;
	for (ai = info; ai; ai = ai->ai_next) {
		struct sockaddr_in *sin = (struct sockaddr_in*)ai->ai_addr;
		int ret = ::connect(fd, (const struct sockaddr*)sin,
				sizeof(struct sockaddr_in));
		cout << "Connect: ret=" << ret << endl;
		m_fd = fd;
		break;
	}
	freeaddrinfo(info);
}

void
HttpRequest::send()
{
	cout << "SEND [" << m_data << "]" << endl;

	string::iterator i;
	for (i = m_data.begin(); i != m_data.end(); )
	{
		int sent = m_connection.safe_write(m_fd, &(*i), distance(i, m_data.end()));
		cerr << "sent: " << sent << " bytes" << endl;
		if (sent <= 0) {
			// TODO: handle
			close(m_fd);
			return;
		}
		i += sent;
	}
}

void
HttpRequest::read_reply(HttpReply &reply)
{
	cout << "READ_REPLY" << endl;
	close(m_fd);
}
