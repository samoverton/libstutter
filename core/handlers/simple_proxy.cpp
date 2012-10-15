#include "simple_proxy.h"
#include "../http/proxy.h"
#include <iostream>

using namespace std;
using http::Proxy;

SimpleProxyHandler::SimpleProxyHandler(http::Connection &cx)
	: BaseHandler(cx)
{
}

void
SimpleProxyHandler::handle(const http::Request &req, http::Reply &reply)
{
	// cout << "SimpleProxyHandler" << endl;
	send_to("bea", req, reply);
	// cout << "We have the full reply from bea, return it to original client" << endl;

	// if (reply.code() != 200 && reply.code() != 204)
	// 	return;
	// send_to("zoe", req, reply);
}

void
SimpleProxyHandler::send_to(string host, const http::Request &req, http::Reply &reply)
{
	Proxy p(connection(), req);
	p.send(host, reply);
}
