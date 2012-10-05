#include "reply.h"
#include "connection.h"
#include <sstream>
#include <iostream>

using namespace std;
using http::Reply;
using http::Connection;

Reply::Reply(Connection &connection)
	: m_connection(connection)
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
Reply::add_header(string key, string val)
{
	m_headers.insert(make_pair(key, val));
}

void
Reply::add_header(string key, int val)
{
	stringstream ss;
	ss << val;
	
	add_header(key, ss.str());
}

void
Reply::add_body(const char *p, size_t sz)
{
	m_body.insert(m_body.end(), p, p+sz);
}

void
Reply::prepare()
{
	add_header("Content-Length", m_body.size());

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
	m_data.insert(m_data.end(), m_body.begin(), m_body.end());
}

void
Reply::reset()
{
	m_headers.clear();
	m_code = 200;
	m_status = "OK";
	m_data.clear();
	m_body.clear();

	add_header("Connection", "keep-alive");
}

short
Reply::code() const
{
	return m_code;
}

Reply::iterator
Reply::begin() const
{
	return m_data.begin();
}

Reply::iterator
Reply::end() const
{
	return m_data.end();
}

