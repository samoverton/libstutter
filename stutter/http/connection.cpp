#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>

#include <stutter/http/connection.h>
#include <stutter/handlers/base.h>
#include <stutter/log.h>
#include <stutter/server.h>

#include <iostream>
using namespace std;

using http::Connection;
using http::Reply;
using http::Parser;

#define READ_BUFFER_SIZE 4096

bool
_process(void *self)
{
	Connection *cx = reinterpret_cast<Connection*>(self);
	return cx->process();
}

Connection::Connection(Server &server, int fd)
	: m_server(server)
	, m_fd(fd)
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

// called by the parser when a request is ready to be processed
bool
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
	if (!send(m_reply)) { // TODO: handle this better
		return false;
	}

	// reset objects for next request
	m_request.reset();
	m_reply.reset();
	m_parser.reset();
	return true;
}

bool
Connection::send(Reply &r)
{
	r.prepare();

	return send_headers(r)
		&& send_buffered_body(m_fd, r.body());
}

bool
Connection::send_headers(Reply &r)
{
	Reply::iterator i;
	for (i = r.begin(); i != r.end(); )
	{
		int sent = safe_write(fd(), &(*i), distance(i, r.end()));
		if (sent <= 0) {
			Log::get(Log::DEBUG) << "Could not send response to client" << endl;
			return false;
		}
		i += sent;
	}
	return true;
}

bool
Connection::send_100_continue()
{
	char hdr[] = "HTTP/1.1 100 Continue\r\n\r\n";
	return send_raw(fd(), hdr, sizeof(hdr)-1);
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

		// add data to the parser and possibly handle a full request
		Parser::Error e = m_parser.add(buffer, (size_t)recvd);
		switch (e) {
			case Parser::PARSE_NEED_100_CONTINUE:
				send_100_continue();
				break;

			case Parser::PARSE_FAILURE:
				return (int)HALT;

			default:
				break;
		}
	}

	return (int)HALT;
}

int
Connection::fd() const
{
	return m_fd;
}

Server &
Connection::server()
{
	return m_server;
}

