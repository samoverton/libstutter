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
	, m_require_100(false)
	, m_error(NOT_EXECUTED)
{}

Request::Request(const Request &request)
	: Message(request)
	, m_verb(request.m_verb)
	, m_url(request.m_url)
	, m_require_100(request.m_require_100)
	, m_error(request.m_error)
{
}

Request::~Request()
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

bool
Request::send_continue()
{
	char hdr[] = "HTTP/1.1 100 Continue\r\n\r\n";
	return send_raw(m_connection.watched_fd(),
			hdr, sizeof(hdr)-1);
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

namespace http {
void
_done(void *self)
{
	Request *req = reinterpret_cast<Request*>(self);
	req->error(http::Request::SUCCESS);
}
}


Request::Error
Request::send(Reply &reply)
{
	prepare();

	reply.reset();
	Parser parser(Parser::RESPONSE, &reply,
			_done, reinterpret_cast<void*>(this));


	connect() && send_data(parser, reply) && read_reply(parser, reply);
	release_socket();
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
	if (get_header("Expect") == "100-continue") {
		m_require_100 = true;
	} else {
		m_data.insert(m_data.end(), m_body.buffer_begin(), m_body.buffer_end());
	}

	cout << "Sending this to back-end: [" << headers << "]" << endl;
}

bool
Request::connect()
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host);
	return pool.get(m_fd);
}

bool
Request::send_data(Parser &parser, Reply &reply)
{
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host);

	for(int error_count = 0; error_count < MAX_SEND_RETRIES; error_count++)
	{
		if (send_headers() && send_body(parser, reply))
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
	return send_raw(m_fd, &m_data[0], m_data.size());
}

bool
Request::send_raw(int fd, const char *data, size_t sz)
{
	size_t done = 0;
	while (done < sz)
	{
		int sent = m_connection.safe_write(fd, data + done, sz - done);

		if (sent <= 0) {
			// TODO: log
			return false;
		}
		done += sent;
	}
	return true;
}


bool
Request::send_body(Parser &parser, Reply &reply)
{
	if (m_require_100) { // wait for 100-continue

		read_reply(parser, reply);
		cout << "GOT SOME DATA, status=" << reply.code() << endl;
		if (reply.code() != 100) {
			return false;
		}

		// send first part of the body
		Body::iterator i = m_body.buffer_begin();
		size_t sz = distance(i, m_body.buffer_end());
		if (!send_raw(m_connection.watched_fd(), &(*i), sz))
			return false;

		cout << "sent first " << sz << " bytes of body" << endl;
	}

	bool ret = m_body.send_from_disk(m_connection);
	if (ret)
		cout << "sent more data from disk-buffered body" << endl;
	else
		cout << "failed to send the rest of the body" << endl;
	return ret;
}


bool
Request::read_reply(Parser &parser, Reply &reply)
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
			m_connection.yield((int)Connection::HALT);
		}

		bool success = parser.add(buffer, (size_t)recvd);
		if (!success) {
			cout << "failed to add " << recvd << " bytes" << endl;
			m_connection.yield((int)Connection::HALT);
		}
	}

	return true;
}

void
Request::release_socket()
{
	// put socket back in the pool
	SocketPool &pool = m_connection.server().pool_manager().get_pool(m_host);
	pool.put(m_fd);
}
