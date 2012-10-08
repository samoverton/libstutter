#include "request.h"
#include "connection.h"
#include "../pool.h"
#include "../server.h"

#include <iostream>
#include <sstream>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define MAX_SEND_RETRIES 3

using namespace std;
using http::Request;
using http::Reply;
using http::Connection;
using http::Parser;

Request::Request(Connection &cx)
	: Message(cx)
	, m_verb(GET)
	, m_error(NOT_EXECUTED)
{}

Request::Request(const Request &request)
	: Message(request)
	, m_verb(request.m_verb)
	, m_url(request.m_url)
	, m_error(request.m_error)
{
}

const string&
Request::url() const
{
	return m_url;
}

void
Request::add_url_fragment(const char *at, size_t sz)
{
	m_url.append(at, sz);
}

void
Request::set_host(string host)
{
	m_host = host;
}

void
Request::set_verb(Verb v)
{
	m_verb = v;
}

Request::Verb
Request::verb() const
{
	return m_verb;
}

string
Request::verb_str() const
{
	switch(m_verb) {
		case GET:    return "GET";
		case POST:   return "POST";
		case PUT:    return "PUT";
		case HEAD:   return "HEAD";
		case DELETE: return "DELETE";
		default:     return "";
	}
}

void
Request::reset()
{
	Message::reset();
	m_verb = GET;
	m_url.clear();
	m_host.clear();
	m_error = NOT_EXECUTED;
}

Request::Error
Request::send(Reply &reply)
{
	prepare();
	connect() && send() && read_reply(reply);
	return m_error;
}

void
Request::error(Error e)
{
	m_error = e;
}

void
Request::prepare()
{
	add_header("Host", m_host);
	add_header("Content-Length", m_body.size());

	stringstream ss;
	string crlf("\r\n");
	ss << verb_str() << " " << m_url << " HTTP/1.1" << crlf;

	map<string,string>::const_iterator hi;
	for(hi = m_headers.begin(); hi != m_headers.end(); hi++) {
		ss << hi->first << ": " << hi->second << crlf;
	}
	ss << crlf;

	string headers = ss.str();
	m_data.insert(m_data.end(), headers.begin(), headers.end());

	// add in-memory body buffer
	m_data.insert(m_data.end(), m_body.buffer_begin(), m_body.buffer_end());
}

bool
Request::connect()
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host);
	return pool.get(m_fd);
}

bool
Request::send()
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host);

	for(int error_count = 0; error_count < MAX_SEND_RETRIES; error_count++)
	{
		if (send_headers() && send_body())
			return true;

		// close socket and remove from the pool
		close(m_fd);
		pool.del(m_fd);

		// reconnect and try again
		connect();
	}

	m_error = WRITE_ERROR;
	return false;
}

bool
Request::send_headers()
{
	Message::iterator i;
	for (i = m_data.begin(); i != m_data.end(); )
	{
		size_t sz = distance<Message::iterator>(i, m_data.end());
		int sent = m_connection.safe_write(m_fd, &(*i), sz);

		if (sent <= 0) {
			// TODO: log
			return false;
		}
		i += sent;
	}
	return true;
}


bool
Request::send_body()
{
	return m_body.send_from_disk(m_connection);
}


void
_done(void *self)
{
	Request *req = reinterpret_cast<Request*>(self);
	req->error(http::Request::Error::SUCCESS);
}

bool
Request::read_reply(Reply &reply)
{
	reply.reset();
	Parser parser(Parser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));

	while(m_error != SUCCESS)
	{
		char buffer[1024];
		int recvd = m_connection.safe_read(m_fd, buffer, sizeof(buffer));
		if (recvd <= 0)
			m_connection.yield((int)Connection::Need::HALT);

		bool success = parser.add(buffer, (size_t)recvd);
		if (!success)
			m_connection.yield((int)Connection::Need::HALT);
	}

	// put socket back in the pool
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host);
	pool.put(m_fd);

	return true;
}
