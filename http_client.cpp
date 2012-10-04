#include "http_client.h"
#include "unistd.h"
#include "string.h"

#include <iostream>
using namespace std;

#define READ_BUFFER_SIZE 4096

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

/* http-parser callbacks */

int
_http_on_url_cb(http_parser *p, const char *at, size_t sz)
{
	HttpClient *c = reinterpret_cast<HttpClient*>(p->data);
	c->m_request.add_url_fragment(at, sz); // FIXME?

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

int
_http_on_header_field_cb(http_parser *p, const char *at, size_t sz)
{
	HttpClient *c = reinterpret_cast<HttpClient*>(p->data);
	c->save_last_header();
	c->m_header_key.append(at, sz);
	return 0;
}

int
_http_on_header_value_cb(http_parser *p, const char *at, size_t sz)
{
	HttpClient *c = reinterpret_cast<HttpClient*>(p->data);
	c->m_header_val.append(at, sz);
	c->m_header_gotval = true;
	return 0;
}

int
_http_on_headers_complete_cb(http_parser *p)
{
	HttpClient *c = reinterpret_cast<HttpClient*>(p->data);
	c->save_last_header();
	return 0;
}

void
HttpClient::save_last_header()
{
	if (m_header_gotval) {
		m_request.add_header(m_header_key, m_header_val);
		m_header_key.clear();
		m_header_val.clear();
		m_header_gotval = false;
	}
}

void
HttpClient::reply()
{
	m_reply.set_status(200, "OK");
	m_reply.add_header("Content-Type", "text/plain");
	m_reply.add_header("Connection", "keep-alive");
	m_reply.add_body("hello, world\n", 13);
	m_reply.prepare();

	HttpReply::iterator i;
	for (i = m_reply.begin(); i != m_reply.end(); )
	{
		int sent = safe_write(&(*i), distance(i, m_reply.end()));
		if (sent <= 0)
			yield((int)Need::HALT);
		i += sent;
	}

	// reset reply and parser for next request
	m_reply.reset();
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
	m_parserconf.on_header_field = _http_on_header_field_cb;
	m_parserconf.on_header_value = _http_on_header_value_cb;
	m_parserconf.on_headers_complete = _http_on_headers_complete_cb;

	// http data
	m_header_key.clear();
	m_header_val.clear();
	m_header_gotval = false;
}

int
HttpClient::safe_read(char *p, size_t sz)
{
	yield((int)Need::READ);
	return read(m_fd, p, sz);
}

int
HttpClient::safe_write(const char *p, size_t sz)
{
	yield((int)Need::WRITE);
	return write(m_fd, p, sz);
}

int
HttpClient::exec()
{
	while(true)
	{
		char buffer[READ_BUFFER_SIZE];
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
