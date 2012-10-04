#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "coroutine.h"
#include "http-parser/http_parser.h"
#include "http_request.h"
#include "http_reply.h"

#include <event.h>
#include <string>
#include <map>

class Server;

class HttpConnection : public Coroutine {
public:

	HttpConnection(Server &server, int fd);
	virtual ~HttpConnection();
	virtual int exec();

	int watched_fd() const;
	struct event *event();
	Server &server();

public:
	typedef enum {READ, WRITE, HALT} Need;


private:
	void process();

	// HTTP
	void configure_http_parser();
	void save_last_header();

	// yielding IO
	int safe_read (int fd, char *p, size_t sz);
	int safe_write(int fd, const char *p, size_t sz);

private:
	Server &m_server;
	int m_fd;
	int m_watched_fd;
	struct event m_ev;

	HttpRequest m_request;
	HttpReply   m_reply;

	// parsing
	struct http_parser m_parser;
	struct http_parser_settings m_parserconf;
	std::string m_header_key;
	std::string m_header_val;
	bool m_header_gotval;

friend int _http_on_url_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_message_complete_cb(http_parser *p);
friend int _http_on_header_field_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_header_value_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_headers_complete_cb(http_parser *p);
};

#endif // HTTP_CONNECTION_H
