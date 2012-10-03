#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "coroutine.h"
#include "http-parser/http_parser.h"
#include <event.h>
#include <string>
#include <map>

class Server;

class HttpClient : public Coroutine {
public:

	HttpClient(Server &server, int fd);
	virtual ~HttpClient();
	virtual int exec();

	int fd() const;
	struct event *event();
	Server &server();

public:
	typedef enum {READ, WRITE, HALT} Need;


private:
	void reply();

	// HTTP
	void configure_http_parser();
	void save_last_header();

	// yielding IO
	int safe_read (char *p, size_t sz);
	int safe_write(char *p, size_t sz);

private:
	Server &m_server;
	int m_fd;
	struct event m_ev;

	// http request
	struct http_parser m_parser;
	struct http_parser_settings m_parserconf;

	std::string m_url;
	std::string m_header_key;
	std::string m_header_val;
	bool m_header_gotval;
	std::map<std::string, std::string> m_headers;


friend int _http_on_url_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_message_complete_cb(http_parser *p);
friend int _http_on_header_field_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_header_value_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_headers_complete_cb(http_parser *p);
};

#endif // HTTP_CLIENT_H
