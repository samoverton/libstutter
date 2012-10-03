#include "http_client.h"
#include "unistd.h"
#include "string.h"

#include <iostream>
using namespace std;

HttpClient::HttpClient(Server &server, int fd)
	: m_server(server)
	, m_fd(fd)
{
	configure_http_parser();
}

HttpClient::~HttpClient()
{
	close(m_fd);
}

int
_http_on_url_cb(http_parser *p, const char *at, size_t sz)
{
	/*
	cout << "URL fragment: [";
	cout.write(at, sz);
	cout << "]" << endl;
	*/
	return 0;
}

int
_http_on_message_complete_cb(http_parser *p)
{
	// cout << "Message complete" << endl;
	HttpClient *c = reinterpret_cast<HttpClient*>(p->data);

	c->reply();

	return 0;
}

void
HttpClient::reply()
{
	char data[] = "HTTP/1.1 200 OK\r\n"
		"Content-Length: 13\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"hello, world\n";

	size_t sz = sizeof(data)-1;
	int done = 0;
	while (done < sz)
	{
		int sent = safe_write(data + done, sz - done);
		if (sent <= 0)
			yield((int)Need::HALT);
		done += sent;
	}

	// reset parser for next request
	configure_http_parser();
}

void
HttpClient::configure_http_parser()
{
	// parser
	http_parser_init(&m_parser, HTTP_REQUEST);
	m_parser.data = reinterpret_cast<void*>(this);

	// callback config
	memset(&m_parserconf, 0, sizeof(m_parserconf));
	m_parserconf.on_url = _http_on_url_cb;
	m_parserconf.on_message_complete = _http_on_message_complete_cb;
}

int
HttpClient::safe_read(char *p, size_t sz)
{
	yield((int)Need::READ);
	return read(m_fd, p, sz);
}

int
HttpClient::safe_write(char *p, size_t sz)
{
	yield((int)Need::WRITE);
	return write(m_fd, p, sz);
}

int
HttpClient::exec()
{
	while(true)
	{
		char buffer[4096];
		int recvd = safe_read(buffer, sizeof(buffer));
		if (recvd <= 0)
			return (int)Need::HALT;

		size_t nparsed = http_parser_execute(&m_parser,
				&m_parserconf, buffer, (size_t)recvd);

		if (nparsed != (size_t)recvd)
			return (int)Need::HALT;
	}

	return (int)Need::HALT;
}

int
HttpClient::fd() const
{
	return m_fd;
}

struct event *
HttpClient::event()
{
	return &m_ev;
}

Server &
HttpClient::server()
{
	return m_server;
}
