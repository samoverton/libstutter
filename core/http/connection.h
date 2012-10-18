#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "core/coroutine.h"
#include <core/http/parser.h>
#include <core/http/request.h>
#include <core/http/reply.h>

#include <event.h>
#include <string>
#include <map>

class Server;

namespace http {

class Connection : public Coroutine {
public:

	Connection(Server &server, int fd);
	virtual ~Connection();
	virtual int exec();

	int watched_fd() const;
	void watch_fd(int fd);
	struct event *event();
	Server &server();

	enum Need {READ, WRITE, HALT};

	// yielding IO
	int safe_read (int fd, char *p, size_t sz);
	int safe_write(int fd, const char *p, size_t sz);
	bool send_raw(int fd, const char *data, size_t sz);

	int safe_read (char *p, size_t sz);
	int safe_write(const char *p, size_t sz);
	int safe_sendfile(int in_fd, off_t *offset, size_t count); // sendfile
	void process(); // callback

private:
	void process_error();

private:
	Server &m_server;
	int m_fd;
	int m_watched_fd;
	struct event m_ev;

	http::Request m_request;
	http::Reply   m_reply;
	http::Parser  m_parser;

friend void _process(void*);
};
}

#endif // HTTP_CONNECTION_H
