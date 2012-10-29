#include <stutter/http/reply.h>
#include <stutter/log.h>
#include <stutter/io/strategy.h>

#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using http::Reply;

Reply::Reply() : Message()
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

const string&
Reply::status() const
{
	return m_status;
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
