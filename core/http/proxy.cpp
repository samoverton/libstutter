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
	, m_done(false)
{

}

bool
Proxy::failure(const string &host, Error e)
{
	m_error = e;
	release_socket(host);
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
Proxy::send(const string host, Reply &reply)
{
	Request r(m_request);
	r.add_header("Host", host);

	// build headers buffer
	r.prepare();

	// connect
	if (!connect(host)) {
		return failure(host, CONNECTION_ERROR);
	}

	if (!send_headers(r)) {
		return failure(host, WRITE_ERROR);
	}

	if (r.require_100_continue() && !wait_for_100()) {
		return failure(host, READ_ERROR);
	}

	if (!send_body(r)) {
		return failure(host, WRITE_ERROR);
	}

	// create an HTTP parser for the response
	Parser parser(Parser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));

	if (!read_reply(parser, reply)) {
		return failure(host, READ_ERROR);
	}

	release_socket(host);

	// cout << "Got the reply" << endl;
	return true;
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
Proxy::wait_for_100()
{
	Reply tmp(m_connection);
	Parser parser(Parser::RESPONSE, &tmp,
			_done, reinterpret_cast<void*>(this));

	if (!read_reply(parser, tmp)) {
		cerr << "Could not read reply while waiting for 100-continue" << endl;
		return false;
	}

	cerr << "tmp.code() = " << tmp.code() << endl;

	// check that we've received a 100-continue
	if (tmp.code() == 100) {
		parser.reset(); // reset as we'll get a full answer later
		cerr << "Got 100-continue" << endl;
		return true;
	}

	return false;
}

bool
Proxy::send_body(Request &r)
{
	Body &body = r.body();
	// cout << "Send body (" << body.size() << " bytes)" << endl;
	if (m_request.require_100_continue()) {
		// send first part of the body
		Body::iterator i = body.buffer_begin();
		size_t sz = distance(i, body.buffer_end());
		if (!m_connection.send_raw(m_fd, &(*i), sz))
			return false;

		// cout << "sent first " << sz << " bytes of body" << endl;
	}

	// send disk-backed body
	return body.send_from_disk(m_connection);
}

bool
Proxy::read_reply(Parser &parser, Reply &reply)
{
	// cout << "read reply on socket " << m_fd << "..." << endl;
	m_done = false;
	while(!m_done)
	{
		char buffer[1024];
		int recvd = m_connection.safe_read(m_fd, buffer, sizeof(buffer));
		// cout << "recvd = " << recvd << endl;
		if (recvd <= 0) {
			// TODO: log
			// cout << "Error reading from " << m_fd << endl;
			return false;
		}

		bool success = parser.add(buffer, (size_t)recvd);
		if (!success) {
			// cout << "failed to add " << recvd << " bytes to parser" << endl;
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
