#include <stutter/http/client.h>
#include <stutter/http/request.h>
#include <stutter/http/reply.h>
#include <stutter/http/parser.h>
#include <stutter/pool.h>
#include <stutter/server.h>
#include <stutter/log.h>
#include <stutter/io/strategy.h>

#include <iostream>

using namespace std;

using http::Client;
using http::Reply;

Client::Client(IOStrategy &io, const string host, short port)
	: m_fd(-1)
	, m_io(io)
	, m_error(NOT_EXECUTED)
	, m_done(false)
	, m_host(host)
	, m_port(port)
{

}

bool
Client::failure(Error e)
{
	m_error = e;
	release_socket();
	return false;
}

namespace http {
void
_done(void *self)
{
	Client *c = reinterpret_cast<Client*>(self);
	c->on_msg_complete();
}
}

void
Client::on_msg_complete()
{
	m_done = true;
}

void
Client::error(Error e)
{
	m_error = e;
}


bool
Client::send(Request &req, Reply &reply)
{
	m_done = false;

	req.set_host(m_host);

	// build headers buffer
	req.prepare();

	// connect
	if (!connect()) {
		return failure(CONNECTION_ERROR);
	}

	if (!send_headers(req)) {
		return failure(WRITE_ERROR);
	}

	if (req.require_100_continue() && !wait_for_100()) {
		return failure(READ_ERROR);
	}

	if (!send_body(req)) {
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
Client::connect()
{
	SocketPool &pool = PoolManager::get_pool(m_host, m_port); // TODO: remove the singleton
	return pool.get(m_fd);
}

bool
Client::send_headers(Request &r)
{
	const Message::iterator begin = r.begin(), end = r.end();
	return m_io.send_raw(m_fd, &(*begin), distance(begin, end));
}

bool
Client::wait_for_100()
{
	Reply tmp;
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
Client::send_body(Request &r)
{
	const Body &body = r.body();
	if (r.require_100_continue()) { // send first part of the body
		Body::iterator i = body.buffer_begin();
		size_t sz = distance(i, body.buffer_end());
		if (!m_io.send_raw(m_fd, &(*i), sz))
			return false;
	}

	// send disk-backed body
	return m_io.send_buffered_body(m_fd, body);
}

bool
Client::read_reply(Parser &parser)
{
	m_done = false;
	while(!m_done)
	{
		char buffer[1024];
		int recvd = m_io.safe_read(m_fd, buffer, sizeof(buffer));
		if (recvd <= 0) {
			Log::get(Log::DEBUG) << "Error reading from fd " << m_fd << endl;
			return false;
		}

		Parser::Error e = parser.add(buffer, (size_t)recvd);
		if (e != Parser::PARSE_OK) {
			Log::get(Log::DEBUG) << "Failed to add " << recvd
                                 << " bytes to HTTP parser" << endl;
			return false;
		}
	}

	return true;
}

void
Client::release_socket()
{
	// put socket back in the pool
	SocketPool &pool = PoolManager::get_pool(m_host, m_port); // TODO: remove the singleton
	pool.put(m_fd);
}
