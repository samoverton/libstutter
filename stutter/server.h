#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <sstream>
#include <map>
#include <event.h>

#include <stutter/http/connection.h>
#include <stutter/pool.h>
#include <stutter/dispatcher.h>
#include <stutter/handlers/base.h>

class Server {
public:

	Server(std::string host, short port);
	void start();

	// options
	typedef enum {OPT_DAEMONIZE, OPT_USER, OPT_GROUP} Option;
	void option(Option o, std::string val);
	const std::string option(Option o) const;

	PoolManager &pool_manager();
	Dispatcher &router();
	void resume(YieldingIOStrategy *io);

private:
	int setup_socket() const;
	void register_connection(YieldingIOStrategy *io, short event);
	void daemonize();
	bool drop_privileges();

private:
	std::string m_host;
	short m_port;
	int m_fd;

	PoolManager m_poolmgr;
	Dispatcher  m_router;

	std::map<Option, std::string> m_options;

	// libevent
	struct event_base *m_base;
	struct event       m_ev;

friend void  _on_possible_accept(int, short, void *);
friend void _on_connection_event(int, short, void *);

};

#endif // SERVER_H
