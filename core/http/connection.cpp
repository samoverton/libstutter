#include "connection.h"
#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>

#include "../handlers/base.h"
#include "../handlers/hello.h"
#include "../handlers/simple_proxy.h"
#include "../handlers/file.h"

#include <iostream>
using namespace std;

using http::Connection;
using http::Reply;
using http::Parser;

#define READ_BUFFER_SIZE 4096

void
_process(void *self)
{
	Connection *cx = reinterpret_cast<Connection*>(self);
	cx->process();
}

Connection::Connection(Server &server, int fd)
	: m_server(server)
	, m_fd(fd)
	, m_watched_fd(fd)
	, m_request(*this)
	, m_reply(*this)
	, m_parser(Parser::REQUEST, &m_request,
			_process, static_cast<void*>(this))
{
}

Connection::~Connection()
{
	close(m_fd);
}

void
Connection::process()
{
	// use custom handler to build reply
	// HelloHandler h(*this);
	SimpleProxyHandler h(*this);
	// FileHandler h(*this);
	h.handle(m_request, m_reply);

	// respond to client
	m_reply.send();

	// reset objects for next request
	m_request.reset();
	m_reply.reset();
	m_parser.reset();
}


int
Connection::safe_read(int fd, char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)READ);
	return read(fd, p, sz);
}

int
Connection::safe_read (char *p, size_t sz)
{
	return safe_read(m_fd, p, sz);
}

int
Connection::safe_write(int fd, const char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)WRITE);
	return write(fd, p, sz);
}

int
Connection::safe_write(const char *p, size_t sz)
{
	return safe_write(m_fd, p, sz);
}

int
Connection::safe_sendfile(int in_fd, off_t *offset, size_t count)
{
	yield((int)WRITE);
	return sendfile(m_watched_fd, in_fd, offset, count);
}

int
Connection::exec()
{
	while(true)
	{
		char buffer[READ_BUFFER_SIZE];
		int recvd = safe_read(m_fd, buffer, sizeof(buffer));
		if (recvd <= 0)
			return (int)HALT;

		bool success = m_parser.add(buffer, (size_t)recvd);
		if (!success)
			return (int)HALT;
	}

	return (int)HALT;
}

int
Connection::watched_fd() const
{
	return m_watched_fd;
}

void
Connection::watch_fd(int fd)
{
	m_watched_fd = fd;
}

struct event *
Connection::event()
{
	return &m_ev;
}

Server &
Connection::server()
{
	return m_server;
}

bool
Connection::send_raw(int fd, const char *data, size_t sz)
{
	size_t done = 0;
	while (done < sz)
	{
		int sent = safe_write(fd, data + done, sz - done);

		if (sent <= 0) {
			// TODO: log
			return false;
		}
		done += sent;
	}
	return true;
}

