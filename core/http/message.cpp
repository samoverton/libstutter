#include "message.h"
#include <sstream>

using namespace std;
using http::Message;

void
Message::add_header(string key, string val)
{
	m_headers.insert(make_pair(key, val));
}

void
Message::add_header(string key, int val)
{
	stringstream ss;
	ss << val;
	
	add_header(key, ss.str());
}

void
Message::add_body(const char *p, size_t sz)
{
	m_body.insert(m_body.end(), p, p+sz);
}

void
Message::reset()
{
	m_headers.clear();
	m_data.clear();
	m_body.clear();
}

Message::iterator
Message::begin() const
{
	return m_data.begin();
}

Message::iterator
Message::end() const
{
	return m_data.end();
}

