#include "server.h"
#include <iostream>

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


using namespace std;

Server::Server(string host, short port)
	: m_host(host)
	, m_port(port)
{
	m_base = event_base_new();
}

int
Server::setup_socket() const
{
	int reuse = 1;
	struct sockaddr_in addr;
	int fd, ret;

	memset(&addr, 0, sizeof(addr));
#if defined __BSD__
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);

	addr.sin_addr.s_addr = inet_addr(m_host.c_str());

	/* this sad list of tests could use a Maybe monad... */

	/* create socket */
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == fd) {
		/*syslog(LOG_ERR, "Socket error: %m\n");*/
		return -1;
	}

	/* reuse address if we've bound to it before. */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
				sizeof(reuse)) < 0) {
		/* syslog(LOG_ERR, "setsockopt error: %m\n"); */
		return -1;
	}

	/* set socket as non-blocking. */
	ret = fcntl(fd, F_SETFD, O_NONBLOCK);
	if (0 != ret) {
		/* syslog(LOG_ERR, "fcntl error: %m\n"); */
		return -1;
	}

	/* bind */
	ret = ::bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (0 != ret) {
		/* syslog(LOG_ERR, "Bind error: %m\n"); */
		return -1;
	}

	/* listen */
	ret = listen(fd, SOMAXCONN);
	if (0 != ret) {
		/* syslog(LOG_DEBUG, "Listen error: %m\n"); */
		return -1;
	}

	/* there you go, ready to accept! */
	return fd;
}

void
Server::resume_client(HttpClient *c)
{
	HttpClient::Need n = (HttpClient::Need)c->resume();
	switch(n) {
		case HttpClient::Need::READ:
			register_client(c, EV_READ);
			break;

		case HttpClient::Need::WRITE:
			register_client(c, EV_WRITE);
			break;

		case HttpClient::Need::HALT:
			cout << "delete client " << c->fd() << endl;
			delete c;
			break;
	}
}

void
_on_client_event(int fd, short event, void *ptr)
{
	HttpClient *c = reinterpret_cast<HttpClient*>(ptr);
	Server &s = c->server();

	s.resume_client(c);
}

void
Server::register_client(HttpClient *c, short event)
{
	struct event *ev = c->event();
	event_set(ev, c->fd(), event, _on_client_event, c);
	event_base_set(m_base, ev);
	event_add(ev, 0); // TODO: check return code
}

void
_on_possible_accept(int fd, short event, void *ptr)
{
	Server *s = reinterpret_cast<Server*>(ptr);

	// accept fd and create client
	struct sockaddr_in addr;
	socklen_t addr_sz = sizeof(addr);
	int client_fd = accept(fd, (struct sockaddr*)&addr, &addr_sz);
	HttpClient *c = new HttpClient(*s, client_fd);

	// resume
	s->resume_client(c);
}

void
Server::start()
{
	/* ignore sigpipe */
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif	
	m_fd = setup_socket(); // TODO: check return code

	/* start http server */
	event_set(&m_ev, m_fd, EV_READ | EV_PERSIST,
			_on_possible_accept, reinterpret_cast<void*>(this));
	event_base_set(m_base, &m_ev);
	event_add(&m_ev, 0); // TODO: check return code

	event_base_dispatch(m_base);
}

