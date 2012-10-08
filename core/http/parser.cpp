#include "parser.h"
#include "message.h"
#include "request.h"
#include "reply.h"

#include <string.h>

using namespace std;
using http::Parser;

/* http-parser callbacks */

namespace http {
int
_http_on_url_cb(http_parser *p, const char *at, size_t sz)
{
	Parser *parser = reinterpret_cast<Parser*>(p->data);
	parser->add_url_fragment(at, sz);
	return 0;
}

int
_http_on_message_complete_cb(http_parser *p)
{
	Parser *parser = reinterpret_cast<Parser*>(p->data);
	parser->callback();
	return 0;
}

int
_http_on_header_field_cb(http_parser *p, const char *at, size_t sz)
{
	Parser *parser = reinterpret_cast<Parser*>(p->data);
	parser->save_last_header();
	parser->m_header_key.append(at, sz);
	return 0;
}

int
_http_on_header_value_cb(http_parser *p, const char *at, size_t sz)
{
	Parser *parser = reinterpret_cast<Parser*>(p->data);
	parser->m_header_val.append(at, sz);
	parser->m_header_gotval = true;
	return 0;
}

int
_http_on_body_cb(http_parser *p, const char *at, size_t sz)
{
	Parser *parser = reinterpret_cast<Parser*>(p->data);
	parser->add_body_fragment(at, sz);
	return 0;
}

int
_http_on_headers_complete_cb(http_parser *p)
{
	Parser *parser = reinterpret_cast<Parser*>(p->data);
	parser->save_last_header();
	return 0;
}
}

void
Parser::save_last_header()
{
	if (m_header_gotval) {
		m_msg->add_header(m_header_key, m_header_val);

		m_header_key.clear();
		m_header_val.clear();
		m_header_gotval = false;
	}

	if (m_mode == REQUEST && m_request->get_header("Expect") == "100-continue")
		m_request->send_continue();
}

void
Parser::add_url_fragment(const char *p, size_t sz)
{
	if (m_mode == REQUEST)
		m_request->add_url_fragment(p, sz);
}

void
Parser::add_body_fragment(const char *at, size_t sz)
{
	m_msg->add_body(at, sz);
}

void
Parser::callback()
{
	if (m_mode == REQUEST) { // Verb
		switch(m_parser.method) {
			case HTTP_GET:    m_request->set_verb(Request::Verb::GET);    break;
			case HTTP_POST:   m_request->set_verb(Request::Verb::POST);   break;
			case HTTP_PUT:    m_request->set_verb(Request::Verb::PUT);    break;
			case HTTP_HEAD:   m_request->set_verb(Request::Verb::HEAD);   break;
			case HTTP_DELETE: m_request->set_verb(Request::Verb::DELETE); break;
			default: break;
		}
	}
	m_fun(m_fun_data);
}

Parser::Parser(Mode m, http::Request *request, void (*fun)(void*), void *ptr)
	: m_mode(m)
	, m_request(request)
	, m_reply(0)
	, m_msg(m_request)
	, m_fun(fun)
	, m_fun_data(ptr)
{
	reset();
}

Parser::Parser(Mode m, http::Reply *reply, void (*fun)(void*), void *ptr)
	: m_mode(m)
	, m_request(0)
	, m_reply(reply)
	, m_msg(m_reply)
	, m_fun(fun)
	, m_fun_data(ptr)
{
	reset();
}

bool
Parser::add(const char *p, size_t sz)
{
	size_t nparsed = http_parser_execute(&m_parser, &m_parserconf, p, sz);
	return (nparsed == sz);
}

void
Parser::reset()
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

