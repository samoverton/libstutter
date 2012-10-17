#include "reply.h"
#include "connection.h"
#include "../log.h"
#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using http::Reply;
using http::Connection;

Reply::Reply(Connection &cx)
	: Message(cx)
{
	reset();
}

Reply::~Reply()
{
}

void
Reply::set_status(short code, std::string status)
{
	m_code = code;
	m_status = status;
}

void
Reply::prepare()
{
	// remove chunked header
	if (get_header("Transfer-Encoding") == "chunked")
		del_header("Transfer-Encoding");

	add_header(Message::ContentLength, m_body.size());

	stringstream ss;
	string crlf("\r\n");
	ss << "HTTP/1.1 " << m_code << " " << m_status << crlf;

	map<string,string>::const_iterator hi;
	for(hi = m_headers.begin(); hi != m_headers.end(); hi++) {
		ss << hi->first << ": " << hi->second << crlf;
	}
	ss << crlf;

	m_data.clear();
	string headers = ss.str();
	m_data.reserve(headers.size() + m_body.size());
	m_data.insert(m_data.end(), headers.begin(), headers.end());

	// insert in-memory part of the body
	m_data.insert(m_data.end(), m_body.buffer_begin(), m_body.buffer_end());
}

void
Reply::reset()
{
	Message::reset();

	set_status(200, "OK");
	add_header(Message::Connec, Message::KeepAlive);
}

short
Reply::code() const
{
	return m_code;
}

bool
Reply::send()
{
	prepare();

	return send_headers()
		&& m_body.send_from_disk(m_connection);
}

bool
Reply::set_file(const std::string filename)
{
	if (!m_body.set_file(filename)) {
		return false;
	}

	add_header(Message::ContentLength, m_body.size());
	return true;
}

bool
Reply::send_headers()
{
	iterator i;
	for (i = begin(); i != end(); )
	{
		int sent = m_connection.safe_write(&(*i), distance(i, end()));
		if (sent <= 0) {
			Log::get(Log::DEBUG) << "Could not send response to client" << endl;
			return false;
		}
		i += sent;
	}
	return true;
}

