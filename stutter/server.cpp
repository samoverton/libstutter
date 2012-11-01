#include <stutter/server.h>
#include <stutter/log.h>
#include <stutter/handlers/error.h>

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstdlib>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <iostream>

using namespace std;
using http::Connection;

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
		Log::get(Log::ERROR) << "Socket error: " << strerror(errno) << endl;
		return -1;
	}

	/* reuse address if we've bound to it before. */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
				sizeof(reuse)) < 0) {
		Log::get(Log::ERROR) << "Setsockopt error: " << strerror(errno) << endl;
		return -1;
	}

	/* set socket as non-blocking. */
	ret = fcntl(fd, F_SETFD, O_NONBLOCK);
	if (0 != ret) {
		Log::get(Log::ERROR) << "Fcntl error: " << strerror(errno) << endl;
		return -1;
	}

	/* bind */
	ret = ::bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (0 != ret) {
		Log::get(Log::ERROR) << "Bind error: " << strerror(errno) << endl;
		return -1;
	}

	/* listen */
	ret = listen(fd, SOMAXCONN);
	if (0 != ret) {
		Log::get(Log::ERROR) << "Listen error: " << strerror(errno) << endl;
		return -1;
	}

	/* there you go, ready to accept! */
	return fd;
}

void
Server::resume(YieldingIOStrategy *io)
{
	YieldingIOStrategy::Need n = (YieldingIOStrategy::Need)io->resume();
	switch(n) {
		case YieldingIOStrategy::READ:
			register_connection(io, EV_READ);
			break;

		case YieldingIOStrategy::WRITE:
			register_connection(io, EV_WRITE);
			break;

		case YieldingIOStrategy::HALT:
			delete io;
			break;
	}
}

PoolManager&
Server::pool_manager()
{
	return m_poolmgr;
}

struct cx_event {
	YieldingIOStrategy *io;
	Server *server;
};

void
_on_connection_event(int fd, short event, void *ptr)
{
	(void)fd;
	(void)event;

	cx_event *cx = reinterpret_cast<cx_event*>(ptr);
	Server *server = cx->server;
	YieldingIOStrategy *io = cx->io;
	delete cx;

	server->resume(io);
}

void
Server::register_connection(YieldingIOStrategy *io, short event)
{
	struct event *ev = io->event();

	// prepare closure
	cx_event *cx = new cx_event;
	cx->io = io;
	cx->server = this;

	event_set(ev, io->watched_fd(), event, _on_connection_event, cx);
	event_base_set(m_base, ev);
	int added = event_add(ev, 0);
	if (added != 0) {
		io->resume(); // the blocking IO call will now fail
	}
}

void
_on_possible_accept(int fd, short event, void *ptr)
{
	(void)event;
	Server *s = reinterpret_cast<Server*>(ptr);

	// accept fd and create connection
	struct sockaddr_in addr;
	socklen_t addr_sz = sizeof(addr);
	int client_fd = accept(fd, (struct sockaddr*)&addr, &addr_sz);
	Connection *c = new Connection(*s, client_fd);

	// resume
	s->resume(c);
}

void
Server::daemonize()
{
	/* fork and stop parent process */
	if (::fork() != 0)
		::exit(0);
	::setsid();

	/* redirect all outputs to /dev/null */
	int fd;
	if ((fd = ::open("/dev/null", O_RDWR, 0)) != -1) {
		::dup2(fd, STDIN_FILENO);
		::dup2(fd, STDOUT_FILENO);
		::dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			::close(fd);
	}
}

bool
Server::drop_privileges()
{
	struct passwd *userp = 0;
	string user = option(OPT_USER);
	if (!user.empty()) {
		userp = getpwnam(user.c_str());
		if (!userp) {
			Log::get(Log::ERROR) << "Unknown user \"" << user << "\"" << endl;
			return false;
		}
	}

	struct group *groupp = 0;
	string group = option(OPT_GROUP);
	if (!group.empty()) {
		groupp = getgrnam(group.c_str());
		if (!groupp) {
			Log::get(Log::ERROR) << "Unknown group \"" << group << "\"" << endl;
			return false;
		}
	}

	if (userp && setuid(userp->pw_uid) < 0) {
		Log::get(Log::ERROR) << "Could not setuid to " << userp->pw_uid
			<< " (user \"" << user << "\")" << endl;
		return false;
	}

	if (groupp && setgid(groupp->gr_gid) < 0) {
		Log::get(Log::ERROR) << "Could not setgid to " << groupp->gr_gid
			<< " (group \"" << group << "\")" << endl;
		return false;
	}

	return true;
}

void
Server::start()
{
	/* ignore sigpipe */
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif	
	m_fd = setup_socket();
	if (m_fd < 0) {
		Log::get(Log::ERROR) << "Could not create socket" << endl;
		return;
	}

	if (option(OPT_DAEMONIZE) == "true") {
		daemonize();
	}

	if (!drop_privileges()) {
		Log::get(Log::ERROR) << "Could not drop privileges" << endl;
		return;
	}

	/* start http server */
	event_set(&m_ev, m_fd, EV_READ | EV_PERSIST,
			_on_possible_accept, reinterpret_cast<void*>(this));
	event_base_set(m_base, &m_ev);
	if (event_add(&m_ev, 0) != 0) {
		Log::get(Log::ERROR) << "Failed to add event to event loop" << endl;
		return;
	}

	/* add default catch-all handler */
	router().add("", new ErrorHandler(403, "Forbidden"));

	Log::get(Log::INFO) << "St-t-t-t-stutter has st-t-t-t-started." << endl;
	event_base_dispatch(m_base);
}

Dispatcher &
Server::router()
{
	return m_router;
}

void
Server::option(Option o, string val)
{
	m_options[o] = val;
}

const std::string
Server::option(Option o) const
{
	map<Option, string>::const_iterator it;
	if ((it = m_options.find(o)) == m_options.end())
		return "";
	return it->second;
}
