#include "parser.h"
#include "request.h"
#include "reply.h"

#include <string.h>

using namespace std;

/* http-parser callbacks */

int
_http_on_url_cb(http_parser *p, const char *at, size_t sz)
{
	HttpParser *parser = reinterpret_cast<HttpParser*>(p->data);
	parser->add_url_fragment(at, sz);

	return 0;
}

int
_http_on_message_complete_cb(http_parser *p)
{
	// cout << "Message complete" << endl;
	HttpParser *parser = reinterpret_cast<HttpParser*>(p->data);
	parser->callback();

	return 0;
}

int
_http_on_header_field_cb(http_parser *p, const char *at, size_t sz)
{
	HttpParser *parser = reinterpret_cast<HttpParser*>(p->data);
	parser->save_last_header();
	parser->m_header_key.append(at, sz);
	return 0;
}

int
_http_on_header_value_cb(http_parser *p, const char *at, size_t sz)
{
	HttpParser *parser = reinterpret_cast<HttpParser*>(p->data);
	parser->m_header_val.append(at, sz);
	parser->m_header_gotval = true;
	return 0;
}

int
_http_on_body_cb(http_parser *p, const char *at, size_t sz)
{
	HttpParser *parser = reinterpret_cast<HttpParser*>(p->data);
	parser->add_body_fragment(at, sz);
	return 0;
}

int
_http_on_headers_complete_cb(http_parser *p)
{
	HttpParser *parser = reinterpret_cast<HttpParser*>(p->data);
	parser->save_last_header();
	return 0;
}

void
HttpParser::save_last_header()
{
	if (m_header_gotval) {
		switch(m_mode) {
			case REQUEST:
				m_request->add_header(m_header_key, m_header_val);
				break;

			case RESPONSE:
				m_reply->add_header(m_header_key, m_header_val);
				break;
		}

		m_header_key.clear();
		m_header_val.clear();
		m_header_gotval = false;
	}
}

void
HttpParser::add_url_fragment(const char *p, size_t sz)
{
	if (m_mode == REQUEST)
		m_request->add_url_fragment(p, sz);
}

void
HttpParser::add_body_fragment(const char *at, size_t sz)
{
	if (m_mode == HttpParser::RESPONSE)
		m_reply->add_body(at, sz);
}

void
HttpParser::callback()
{
	m_fun(m_fun_data);
}

HttpParser::HttpParser(Mode m, HttpRequest *request, void (*fun)(void*), void *ptr)
	: m_mode(m)
	, m_request(request)
	, m_reply(0)
	, m_fun(fun)
	, m_fun_data(ptr)
{
	reset();
}

HttpParser::HttpParser(Mode m, HttpReply *reply, void (*fun)(void*), void *ptr)
	: m_mode(m)
	, m_request(0)
	, m_reply(reply)
	, m_fun(fun)
	, m_fun_data(ptr)
{
	reset();
}

bool
HttpParser::add(const char *p, size_t sz)
{
	size_t nparsed = http_parser_execute(&m_parser, &m_parserconf, p, sz);
	return (nparsed == sz);
}

void
HttpParser::reset()
{
	// parser
	http_parser_init(&m_parser, (m_mode == REQUEST ? HTTP_REQUEST : HTTP_RESPONSE));
	m_parser.data = reinterpret_cast<void*>(this);

	// callback config
	memset(&m_parserconf, 0, sizeof(m_parserconf));
	m_parserconf.on_url = _http_on_url_cb;
	m_parserconf.on_message_complete = _http_on_message_complete_cb;
	m_parserconf.on_header_field = _http_on_header_field_cb;
	m_parserconf.on_header_value = _http_on_header_value_cb;
	m_parserconf.on_headers_complete = _http_on_headers_complete_cb;
	m_parserconf.on_body = _http_on_body_cb;

	// http data
	m_header_key.clear();
	m_header_val.clear();
	m_header_gotval = false;
}
