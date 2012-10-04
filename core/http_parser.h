#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "http-parser/http_parser.h"
#include <string>
#include <functional>

class HttpRequest;
class HttpReply;

class HttpParser {
public:
	typedef enum {REQUEST, RESPONSE} Mode;
	HttpParser(Mode m, HttpRequest *request, void (*fun)(void*), void *ptr);
	HttpParser(Mode m, HttpReply *reply, void (*fun)(void*), void *ptr);

	bool add(const char *p, size_t sz);
	void reset();

private:
	void configure_http_parser();
	void save_last_header();
	void add_url_fragment (const char *p, size_t sz);
	void add_body_fragment(const char *at, size_t sz);
	void callback();

private:
	Mode m_mode;
	HttpRequest *m_request;
	HttpReply *m_reply;

	// callback (TODO: use std::function?)
	void (*m_fun)(void*);
	void *m_fun_data;

	struct http_parser m_parser;
	struct http_parser_settings m_parserconf;
	std::string m_header_key;
	std::string m_header_val;
	bool m_header_gotval;

friend int _http_on_url_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_message_complete_cb(http_parser *p);
friend int _http_on_header_field_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_header_value_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_body_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_headers_complete_cb(http_parser *p);
};

#endif // HTTP_PARSER_H
