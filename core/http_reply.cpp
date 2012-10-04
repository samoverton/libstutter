#include "http_reply.h"
#include "http_connection.h"
#include <sstream>
#include <iostream>

using namespace std;

HttpReply::HttpReply(HttpConnection &connection)
	: m_connection(connection)
{
	reset();
}

void
HttpReply::set_status(short code, std::string status)
{
	m_code = code;
	m_status = status;
}

void
HttpReply::add_header(string key, string val)
{
	m_headers.insert(make_pair(key, val));
}

void
HttpReply::add_header(string key, int val)
{
	stringstream ss;
	ss << val;
	
	add_header(key, ss.str());
}

void
HttpReply::add_body(const char *p, size_t sz)
{
	m_body.insert(m_body.end(), p, p+sz);
}

void
HttpReply::prepare()
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
HttpReply::reset()
{
	m_headers.clear();
	m_code = 200;
	m_status = "OK";
	m_data.clear();
	m_body.clear();

	add_header("Connection", "keep-alive");
}

HttpReply::iterator
HttpReply::begin() const
{
	return m_data.begin();
}

HttpReply::iterator
HttpReply::end() const
{
	return m_data.end();
}

