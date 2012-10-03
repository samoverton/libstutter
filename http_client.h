#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "coroutine.h"
#include <event.h>

class Server;

class HttpClient : public Coroutine {
public:

	HttpClient(Server &server, int fd);
	virtual int exec();

	int fd() const;
	struct event *event();
	Server &server();

public:
	typedef enum {READ, WRITE, HALT} Need;


private:
	int safe_read (char *p, size_t sz);
	int safe_write(char *p, size_t sz);

private:
	Server &m_server;
	int m_fd;
	struct event m_ev;
};

#endif // HTTP_CLIENT_H
