#include "reply.h"
#include "connection.h"
#include <sstream>
#include <iostream>

using namespace std;
using http::Reply;
using http::Connection;

Reply::Reply(Connection &connection)
	: Message()
	, m_connection(connection)
{
	reset();
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
	char p[] = "hello, world\n";
	// add_header("Content-Length", m_body.size()); // TODO: restore
	add_header("Content-Length", sizeof(p)-1);

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

	m_data.insert(m_data.end(), p, p+sizeof(p)-1);
	// m_data.insert(m_data.end(), m_body.begin(), m_body.end());
}

void
Reply::reset()
{
	Message::reset();

	m_code = 200;
	m_status = "OK";
	add_header("Connection", "keep-alive");
}

short
Reply::code() const
{
	return m_code;
}
