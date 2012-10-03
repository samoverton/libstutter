#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <event.h>

class Server {
public:

	Server(std::string host, short port);
	void start();

private:
	int setup_socket() const;

private:
	std::string m_host;
	short m_port;
	int m_fd;

	struct event_base *m_base;
	struct event       m_ev;
};

#endif // SERVER_H
