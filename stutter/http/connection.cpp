#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>

#include <stutter/http/connection.h>
#include <stutter/handlers/base.h>
#include <stutter/handlers/hello.h>
#include <stutter/handlers/file.h>
#include <stutter/log.h>
#include <stutter/server.h>

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
Connection::process_error()
{
	switch (m_parser.error()) {
		case Parser::PARSE_BODY_TOO_LARGE:
			m_reply.set_status(413, "Request entity too large");
			break;

		case Parser::PARSE_URI_TOO_LONG:
			m_reply.set_status(414, "Request URI too long");
			break;

		default:
			m_reply.set_status(400, "Bad request");
			break;
	}
}

void
Connection::process()
{
	// check for errors
	if (m_parser.error() != Parser::PARSE_OK) {
		process_error();
	} else {
		// use custom handler to build reply
		BaseHandler *h = m_server.router().get(m_request.url());
		h->handle(*this, m_request, m_reply);
	}

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

		Parser::Error e = m_parser.add(buffer, (size_t)recvd);
		if (e == Parser::PARSE_FAILURE) {
			return (int)HALT;
		}
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
			Log::get(Log::DEBUG) << "Failed to send " << (sz-done)
								 << " bytes to fd " << fd << endl;
			return false;
		}
		done += sent;
	}
	return true;
}

