#include "message.h"
#include <sstream>
#include <iostream>

using namespace std;
using http::Message;
using http::Body;

Message::Message()
{
}
Message::Message(const Message &msg)
	: m_headers(msg.m_headers)
	, m_body(msg.m_body)
{
}

void
Message::add_header(string key, string val)
{
	m_headers[key] = val; // replace
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
	size_t done = 0;
	while (done != sz) {
		ssize_t added = m_body.add(p+done, sz-done);
		if (added <= 0) {
			// TODO: log, propagate
			return;
		}
		done += added;
	}
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

