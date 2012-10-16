#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <event.h>
#include "http/connection.h"
#include "pool.h"
#include <dispatcher.h>
#include <handlers/base.h>

class Server {
public:

	Server(std::string host, short port);
	void start();
	PoolManager &pool_manager();

	// bool add_handler(std::string prefix, BaseHandler *handler);

private:
	int setup_socket() const;
	void register_connection(http::Connection *c, short event);
	void resume_connection(http::Connection *c);

private:
	std::string m_host;
	short m_port;
	int m_fd;

	PoolManager m_poolmgr;
	Dispatcher  m_route;

	// libevent
	struct event_base *m_base;
	struct event       m_ev;

friend void  _on_possible_accept(int, short, void *);
friend void _on_connection_event(int, short, void *);

};

#endif // SERVER_H
