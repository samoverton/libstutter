#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <event.h>
#include "http_connection.h"

class Server {
public:

	Server(std::string host, short port);
	void start();

private:
	int setup_socket() const;
	void register_connection(HttpConnection *c, short event);
	void resume_connection(HttpConnection *c);

private:
	std::string m_host;
	short m_port;
	int m_fd;

	// libevent
	struct event_base *m_base;
	struct event       m_ev;

friend void  _on_possible_accept(int, short, void *);
friend void _on_connection_event(int, short, void *);

};

#endif // SERVER_H
