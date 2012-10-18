#include <stutter/http/proxy.h>
#include <stutter/http/connection.h>
#include <stutter/http/request.h>
#include <stutter/http/reply.h>
#include <stutter/http/parser.h>
#include <stutter/pool.h>
#include <stutter/server.h>
#include <stutter/log.h>

#include <iostream>

using namespace std;

using http::Proxy;
using http::Connection;
using http::Reply;

Proxy::Proxy(Connection &cx, const Request &req, const string &host, short port)
	: m_fd(-1)
	, m_connection(cx)
	, m_request(req)
	, m_error(NOT_EXECUTED)
	, m_done(false)
	, m_host(host)
	, m_port(port)
{

}

bool
Proxy::failure(Error e)
{
	m_error = e;
	release_socket();
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
Proxy::send(Reply &reply)
{
	m_done = false;

	Request r(m_request);
	r.add_header(Message::Host, m_host);

	// build headers buffer
	r.prepare();

	// connect
	if (!connect()) {
		return failure(CONNECTION_ERROR);
	}

	if (!send_headers(r)) {
		return failure(WRITE_ERROR);
	}

	if (r.require_100_continue() && !wait_for_100()) {
		return failure(READ_ERROR);
	}

	if (!send_body(r)) {
		return failure(WRITE_ERROR);
	}

	// create an HTTP parser for the response
	Parser parser(Parser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));

	if (!read_reply(parser)) {
		return failure(READ_ERROR);
	}

	release_socket();

	return true;
}

bool
Proxy::connect()
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host, m_port);
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
		Log::get(Log::INFO) << "Could not read reply "
			"while waiting for 100-continue" << endl;
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
Proxy::release_socket()
{
	// put socket back in the pool
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host, m_port);
	pool.put(m_fd);
}
