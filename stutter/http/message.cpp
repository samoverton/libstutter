#include <stutter/http/message.h>
#include <stutter/io/strategy.h>
#include <stutter/log.h>

#include <sstream>
#include <iostream>

using namespace std;
using http::Message;
using http::Body;

const string Message::Expect = "Expect";
const string Message::ContentLength = "Content-Length";
const string Message::Host = "Host";
const string Message::Connec = "Connection";

const string Message::KeepAlive = "keep-alive";
const string Message::OneHundredContinue = "100-continue";
const string Message::Close = "Close";

Message::Message()
{
}
Message::Message(const Message &msg)
    : m_headers(msg.m_headers)
    , m_body(msg.m_body)
{
}

Message::~Message()
{
}

void
Message::add_header(const string &key, const string &val)
{
    m_headers[key] = val; // replace
}

void
Message::add_header(const string &key, int val)
{
    stringstream ss;
    ss << val;
    
    add_header(key, ss.str());
}

bool
Message::del_header(const string &key)
{
    map<string,string>::iterator it;
    if ((it = m_headers.find(key)) != m_headers.end()) {
        m_headers.erase(it);
        return true;
    }
    return false;
}

void
Message::add_body(const char *p, size_t sz)
{
    size_t done = 0;
    while (done != sz) {
        ssize_t added = m_body.add(p+done, sz-done);
        if (added <= 0) {
            Log::get(Log::DEBUG) << "Could not add "
                << (sz-done) << " bytes to message body" << endl;
            return;
        }
        done += added;
    }
}

string
Message::get_header(const string &key) const
{
    map<string,string>::const_iterator it;
    if ((it = m_headers.find(key)) == m_headers.end())
        return "";

    return it->second;
}

const Body &
Message::body() const
{
    return m_body;
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

