#include "simple_proxy.h"
#include <iostream>

using namespace std;

SimpleProxyHandler::SimpleProxyHandler(http::Connection &cx)
	: BaseHandler(cx)
{
}

void
SimpleProxyHandler::handle(const http::Request &req, http::Reply &reply)
{
	cout << "SimpleProxyHandler, req.body().size() = " << req.body().size() << endl;
	send_to("bea", req, reply);
	cout << "We have the full reply from bea, return it to original client" << endl;
	// if (reply.code() != 200 && reply.code() != 204)
	// 	return;
	// send_to("zoe", req, reply);
}

void
SimpleProxyHandler::send_to(string host, const http::Request &req, http::Reply &reply)
{
	http::Request storage_fw(req);
	storage_fw.set_host(host);
	storage_fw.send(reply);
}
