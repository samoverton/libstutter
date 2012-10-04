#include "http_connection.h"
#include "unistd.h"
#include "string.h"

#include "handlers/base.h"
#include "handlers/hello.h"
#include "handlers/simple_proxy.h"

#include <iostream>
using namespace std;

#define READ_BUFFER_SIZE 4096

HttpConnection::HttpConnection(Server &server, int fd)
	: m_server(server)
	, m_fd(fd)
	, m_watched_fd(fd)
	, m_request(*this)
	, m_reply(*this)
{
	configure_http_parser();
}

HttpConnection::~HttpConnection()
{
	close(m_fd);
}

/* http-parser callbacks */

int
_http_on_url_cb(http_parser *p, const char *at, size_t sz)
{
	HttpConnection *c = reinterpret_cast<HttpConnection*>(p->data);
	c->m_request.add_url_fragment(at, sz); // FIXME?

	return 0;
}

int
_http_on_message_complete_cb(http_parser *p)
{
	// cout << "Message complete" << endl;
	HttpConnection *c = reinterpret_cast<HttpConnection*>(p->data);

	c->process();

	return 0;
}

int
_http_on_header_field_cb(http_parser *p, const char *at, size_t sz)
{
	HttpConnection *c = reinterpret_cast<HttpConnection*>(p->data);
	c->save_last_header();
	c->m_header_key.append(at, sz);
	return 0;
}

int
_http_on_header_value_cb(http_parser *p, const char *at, size_t sz)
{
	HttpConnection *c = reinterpret_cast<HttpConnection*>(p->data);
	c->m_header_val.append(at, sz);
	c->m_header_gotval = true;
	return 0;
}

int
_http_on_headers_complete_cb(http_parser *p)
{
	HttpConnection *c = reinterpret_cast<HttpConnection*>(p->data);
	c->save_last_header();
	return 0;
}

void
HttpConnection::save_last_header()
{
	if (m_header_gotval) {
		m_request.add_header(m_header_key, m_header_val);
		m_header_key.clear();
		m_header_val.clear();
		m_header_gotval = false;
	}
}

void
HttpConnection::process()
{
	// use custom handler to build reply
	// HelloHandler h(*this);
	SimpleProxyHandler h(*this);
	h.handle(m_request, m_reply);

	// pack reply buffer
	m_reply.prepare();

	HttpReply::iterator i;
	for (i = m_reply.begin(); i != m_reply.end(); )
	{
		int sent = safe_write(m_fd, &(*i), distance(i, m_reply.end()));
		if (sent <= 0)
			yield((int)Need::HALT);
		i += sent;
	}

	// reset reply and parser for next request
	m_reply.reset();
	configure_http_parser();
}

void
HttpConnection::configure_http_parser()
{
	// parser
	http_parser_init(&m_parser, HTTP_REQUEST);
	m_parser.data = reinterpret_cast<void*>(this);

	// callback config
	memset(&m_parserconf, 0, sizeof(m_parserconf));
	m_parserconf.on_url = _http_on_url_cb;
	m_parserconf.on_message_complete = _http_on_message_complete_cb;
	m_parserconf.on_header_field = _http_on_header_field_cb;
	m_parserconf.on_header_value = _http_on_header_value_cb;
	m_parserconf.on_headers_complete = _http_on_headers_complete_cb;

	// http data
	m_header_key.clear();
	m_header_val.clear();
	m_header_gotval = false;
}

int
HttpConnection::safe_read(int fd, char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)Need::READ);
	return read(fd, p, sz);
}

int
HttpConnection::safe_write(int fd, const char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)Need::WRITE);
	return write(fd, p, sz);
}

int
HttpConnection::exec()
{
	while(true)
	{
		char buffer[READ_BUFFER_SIZE];
		int recvd = safe_read(m_fd, buffer, sizeof(buffer));
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
HttpConnection::watched_fd() const
{
	return m_watched_fd;
}

void
HttpConnection::watch_fd(int fd)
{
	m_watched_fd = fd;
}

struct event *
HttpConnection::event()
{
	return &m_ev;
}

Server &
HttpConnection::server()
{
	return m_server;
}
