#include "simple_proxy.h"
#include "../http/proxy.h"
#include <iostream>

using namespace std;
using http::Proxy;
using http::Connection;

SimpleProxyHandler::SimpleProxyHandler()
	: BaseHandler()
{
}

void
SimpleProxyHandler::handle(Connection &cx,const http::Request &req, http::Reply &reply)
{
	send_to("bea", cx, req, reply);
	// cout << "We have the full reply from bea, return it to original client" << endl;

	// if (reply.code() != 200 && reply.code() != 204)
	// 	return;
	// send_to("zoe", req, reply);
}

void
SimpleProxyHandler::send_to(string host, Connection &cx, const http::Request &req, http::Reply &reply)
{
	Proxy p(cx, req);
	p.send(host, reply);
}
