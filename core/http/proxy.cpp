#include "proxy.h"
#include "connection.h"
#include "request.h"
#include "reply.h"
#include "parser.h"
#include "../pool.h"
#include "../server.h"
#include "../log.h"

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
	, m_done(false)
{

}

bool
Proxy::failure(const string &host, short port, Error e) // TODO: refactor.
{
	m_error = e;
	release_socket(host, port);
	return false;
}

namespace http {
void
_done(void *self)
{
	Proxy *proxy = reinterpret_cast<Proxy*>(self);
	proxy->on_msg_complete();
}
}

void
Proxy::on_msg_complete()
{
	m_done = true;
}

void
Proxy::error(Error e)
{
	m_error = e;
}


bool
Proxy::send(const string host, short port, Reply &reply)
{
	m_done = false;
	Request r(m_request);
	r.add_header("Host", host);

	// build headers buffer
	r.prepare();

	// connect
	if (!connect(host, port)) {
		return failure(host, port, CONNECTION_ERROR);
	}

	if (!send_headers(r)) {
		return failure(host, port, WRITE_ERROR);
	}

	if (r.require_100_continue() && !wait_for_100()) {
		return failure(host, port, READ_ERROR);
	}

	if (!send_body(r)) {
		return failure(host, port, WRITE_ERROR);
	}

	// create an HTTP parser for the response
	Parser parser(Parser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));

	if (!read_reply(parser)) {
		return failure(host, port, READ_ERROR);
	}

	release_socket(host, port);

	return true;
}

bool
Proxy::connect(const string &host, short port)
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(host, port);
	return pool.get(m_fd);
}

bool
Proxy::send_headers(Request &r)
{
	const Message::iterator begin = r.begin(), end = r.end();
	return m_connection.send_raw(m_fd, &(*begin), distance(begin, end));
}

bool
Proxy::wait_for_100()
{
	Reply tmp(m_connection);
	Parser parser(Parser::RESPONSE, &tmp,
			_done, reinterpret_cast<void*>(this));

	if (!read_reply(parser)) {
		Log::get(Log::INFO) << "Could not read reply while waiting for 100-continue" << endl;
		return false;
	}

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
	const Body &body = r.body();
	if (m_request.require_100_continue()) { // send first part of the body
		Body::iterator i = body.buffer_begin();
		size_t sz = distance(i, body.buffer_end());
		if (!m_connection.send_raw(m_fd, &(*i), sz))
			return false;
	}

	// send disk-backed body
	return body.send_from_disk(m_connection);
}

bool
Proxy::read_reply(Parser &parser)
{
	m_done = false;
	while(!m_done)
	{
		char buffer[1024];
		int recvd = m_connection.safe_read(m_fd, buffer, sizeof(buffer));
		if (recvd <= 0) {
			Log::get(Log::DEBUG) << "Error reading from fd " << m_fd << endl;
			return false;
		}

		bool success = parser.add(buffer, (size_t)recvd);
		if (!success) {
			Log::get(Log::DEBUG) << "Failed to add " << recvd
                                 << " bytes to HTTP parser" << endl;
			return false;
		}
	}

	return true;
}

void
Proxy::release_socket(const string &host, short port)
{
	// put socket back in the pool
	SocketPool &pool = m_connection.server().pool_manager().get_pool(host, port);
	pool.put(m_fd);
}
