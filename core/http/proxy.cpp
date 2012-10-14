#include "proxy.h"
#include "connection.h"
#include "request.h"
#include "reply.h"
#include "parser.h"
#include "../pool.h"
#include "../server.h"

#include <iostream>

using namespace std;

using http::Proxy;
using http::Connection;
using http::Reply;

Proxy::Proxy(Connection &cx, const Request &req)
	: m_fd(-1)
	, m_connection(cx)
	, m_request(req)
	, m_error(NOT_EXECUTED)
{

}

bool
Proxy::failure(Error e)
{
	m_error = e;
	return false;
}

namespace http {
void
_done(void *self)
{
	Proxy *proxy = reinterpret_cast<Proxy*>(self);
	proxy->error(http::Proxy::SUCCESS);
}
}

void
Proxy::error(Error e)
{
	m_error = e;
}


bool
Proxy::send(const string host, Reply &reply)
{
	Request r(m_request);
	r.add_header("Host", host);

	// build headers buffer
	r.prepare();

	// create an HTTP parser for the response
	Parser parser(Parser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));

	// connect
	if (!connect(host)) {
		return failure(CONNECTION_ERROR);
	}

	if (!send_headers(r)) {
		return failure(WRITE_ERROR);
	}

	if (r.require_100_continue() && !wait_for_100(parser)) {
		return failure(READ_ERROR);
	}

	if (!send_body(r)) {
		return failure(WRITE_ERROR);
	}

	if (!read_reply(parser, reply)) {
		return failure(READ_ERROR);
	}

	return false;
}

bool
Proxy::connect(const string &host)
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(host);
	return pool.get(m_fd);
}

bool
Proxy::send_headers(Request &r)
{
	const Message::iterator begin = r.begin(), end = r.end();
	return m_connection.send_raw(m_fd, &(*begin), distance(begin, end));
}

bool
Proxy::wait_for_100(Parser &parser)
{
	Reply tmp(m_connection);
	if (!read_reply(parser, tmp))
		return false;

	// check that we've received a 100-continue
	if (tmp.code() == 100) {
		parser.reset(); // reset as we'll get a full answer later
		return true;
	}

	return false;
}

bool
Proxy::send_body(Request &r)
{
	Body &body = r.body();
	if (m_request.require_100_continue()) {
		// send first part of the body
		Body::iterator i = body.buffer_begin();
		size_t sz = distance(i, body.buffer_end());
		if (!m_connection.send_raw(m_fd, &(*i), sz))
			return false;

		cout << "sent first " << sz << " bytes of body" << endl;
	}

	// send disk-backed body
	return body.send_from_disk(m_connection);
}

bool
Proxy::read_reply(Parser &parser, Reply &reply)
{
	cout << "read reply..." << endl;
	while(m_error != SUCCESS)
	{
		char buffer[1024];
		int recvd = m_connection.safe_read(m_fd, buffer, sizeof(buffer));
		cout << "recvd = " << recvd << endl;
		if (recvd <= 0) {
			// TODO: log
			cout << "Error reading from " << m_fd << endl;
			return false;
		}

		bool success = parser.add(buffer, (size_t)recvd);
		if (!success) {
			cout << "failed to add " << recvd << " bytes to parser" << endl;
			return false;
		}
	}

	return true;
}

void
Proxy::release_socket(const string &host)
{
	// put socket back in the pool
	SocketPool &pool = m_connection.server().pool_manager().get_pool(host);
	pool.put(m_fd);
}
