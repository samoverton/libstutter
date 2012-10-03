#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "coroutine.h"
#include "http-parser/http_parser.h"
#include <event.h>

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

	void configure_http_parser();
	int safe_read (char *p, size_t sz);
	int safe_write(char *p, size_t sz);

private:
	Server &m_server;
	int m_fd;
	struct event m_ev;

	struct http_parser m_parser;
	struct http_parser_settings m_parserconf;

friend int _http_on_url_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_message_complete_cb(http_parser *p);
};

#endif // HTTP_CLIENT_H
