#include "http_request.h"
#include "http_client.h"

using namespace std;

HttpRequest::HttpRequest(HttpClient &client)
	: m_client(client)
{}

HttpRequest::HttpRequest(const HttpRequest &request)
	: m_url(request.m_url)
	, m_headers(request.m_headers)
	, m_client(request.m_client)
{
}

const string&
HttpRequest::url() const
{
	return m_url;
}

void
HttpRequest::add_header(string &key, string &val)
{
	m_headers.insert(make_pair(key, val));
}

void
HttpRequest::add_url_fragment(const char *at, size_t sz)
{
	m_url.append(at, sz);
}
