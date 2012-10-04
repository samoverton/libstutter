#include "http_request.h"

using namespace std;

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
