#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "coroutine.h"
#include "http_parser.h"
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
	void watch_fd(int fd);
	struct event *event();
	Server &server();

public:
	typedef enum {READ, WRITE, HALT} Need;

	// yielding IO
	int safe_read (int fd, char *p, size_t sz);
	int safe_write(int fd, const char *p, size_t sz);

private:
	void process();

private:
	Server &m_server;
	int m_fd;
	int m_watched_fd;
	struct event m_ev;

	HttpRequest m_request;
	HttpReply   m_reply;
	HttpParser  m_parser;

friend void _process(void*);
};

#endif // HTTP_CONNECTION_H
