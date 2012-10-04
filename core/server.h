#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <event.h>
#include "http_client.h"

class Server {
public:

	Server(std::string host, short port);
	void start();

private:
	int setup_socket() const;
	void register_client(HttpClient *c, short event);
	void resume_client(HttpClient *c);

private:
	std::string m_host;
	short m_port;
	int m_fd;

	// libevent
	struct event_base *m_base;
	struct event       m_ev;

friend void _on_possible_accept(int, short, void *);
friend void    _on_client_event(int, short, void *);

};

#endif // SERVER_H
