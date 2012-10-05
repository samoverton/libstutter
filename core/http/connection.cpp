#include "connection.h"
#include "unistd.h"
#include "string.h"

#include "../handlers/base.h"
#include "../handlers/hello.h"
#include "../handlers/simple_proxy.h"

#include <iostream>
using namespace std;
using namespace std::placeholders;

using http::Reply;

#define READ_BUFFER_SIZE 4096

void
_process(void *self)
{
	HttpConnection *cx = reinterpret_cast<HttpConnection*>(self);
	cx->process();
}

HttpConnection::HttpConnection(Server &server, int fd)
	: m_server(server)
	, m_fd(fd)
	, m_watched_fd(fd)
	, m_request(*this)
	, m_reply(*this)
	, m_parser(HttpParser::REQUEST, &m_request,
			_process, static_cast<void*>(this))
{
}

HttpConnection::~HttpConnection()
{
	close(m_fd);
}

void
HttpConnection::process()
{
	// use custom handler to build reply
	// HelloHandler h(*this);
	SimpleProxyHandler h(*this);
	h.handle(m_request, m_reply);

	// pack reply buffer
	m_reply.prepare();

	Reply::iterator i;
	for (i = m_reply.begin(); i != m_reply.end(); )
	{
		int sent = safe_write(m_fd, &(*i), distance(i, m_reply.end()));
		if (sent <= 0)
			yield((int)Need::HALT);
		i += sent;
	}

	// reset objects for next request
	m_request.reset();
	m_reply.reset();
	m_parser.reset();
}


int
HttpConnection::safe_read(int fd, char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)Need::READ);
	return read(fd, p, sz);
}

int
HttpConnection::safe_write(int fd, const char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)Need::WRITE);
	return write(fd, p, sz);
}

int
HttpConnection::exec()
{
	while(true)
	{
		char buffer[READ_BUFFER_SIZE];
		int recvd = safe_read(m_fd, buffer, sizeof(buffer));
		if (recvd <= 0)
			return (int)Need::HALT;

		bool success = m_parser.add(buffer, (size_t)recvd);
		if (!success)
			return (int)Need::HALT;
	}

	return (int)Need::HALT;
}

int
HttpConnection::watched_fd() const
{
	return m_watched_fd;
}

void
HttpConnection::watch_fd(int fd)
{
	m_watched_fd = fd;
}

struct event *
HttpConnection::event()
{
	return &m_ev;
}

Server &
HttpConnection::server()
{
	return m_server;
}
