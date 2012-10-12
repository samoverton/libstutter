#include "proxy.h"
#include "connection.h"
#include "request.h"
#include "reply.h"

using namespace std;

using http::Proxy;
using http::Connection;
using http::Reply;

Proxy::Proxy(Connection &cx, const Request &req)
	: m_connection(cx)
	, m_request(req)
{

}

bool
Proxy::send(const string host, Reply &reply)
{
	return false;
}
