#include <stutter/http/request.h>
#include <stutter/server.h>

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
using http::Parser;

Request::Request()
    : Message()
    , m_verb(GET)
    , m_require_100(false)
{}

Request::Request(const Request &request)
    : Message(request)
    , m_verb(request.m_verb)
    , m_url(request.m_url)
    , m_require_100(request.m_require_100)
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
Request::extract_query_string()
{
    string::size_type first = m_url.find_first_of("?");

    while(first != string::npos) {
        string::size_type next = m_url.find_first_of("&", first+1);

        size_t pair_sz = (next == string::npos ? m_url.size() : next) - first - 1;
        bool is_pair = false;

        // look for equal sign
        for (string::size_type i = 0; i < pair_sz; ++i) {
            if (m_url[first+1+i] == '=') {

                if (i != 0) { // skip arg with empty name
                    m_querystring.insert(make_pair(
                            m_url.substr(first+1, i),
                            m_url.substr(first+1+i+1, pair_sz-i-1)));
                }
                is_pair = true;
                break;
            }
        }
        if (!is_pair) { // arg with empty value
            m_querystring.insert(make_pair(m_url.substr(first+1, next-first-1), ""));
        }

        first = next;
        continue;
    }
}

void
Request::set_url(string url)
{
    m_url = url;
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

const Request::QueryString &
Request::query_string() const
{
    return m_querystring;
}

bool
Request::require_100_continue() const
{
    return m_require_100;
}

void
Request::reset()
{
    Message::reset();
    m_require_100 = false;
    m_verb = GET;
    m_url.clear();
    m_host.clear();
    m_querystring.clear();
}

void
Request::prepare()
{
    add_header(Message::Host, m_host);
    if (m_verb != GET)
        add_header(Message::ContentLength, m_body.size());

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
    if (get_header(Message::Expect) == Message::OneHundredContinue) {
        m_require_100 = true;
    } else {
        m_data.insert(m_data.end(), m_body.buffer_begin(), m_body.buffer_end());
    }
}

