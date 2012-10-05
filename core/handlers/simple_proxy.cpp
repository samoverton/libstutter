#include "simple_proxy.h"

using namespace std;

SimpleProxyHandler::SimpleProxyHandler(http::Connection &cx)
	: BaseHandler(cx)
{
}

void
SimpleProxyHandler::handle(const http::Request &req, http::Reply &reply)
{
	send_to("10.2.129.13", req, reply);
	// if (reply.code() != 200 && reply.code() != 204)
	// 	return;
	send_to("zoe", req, reply);
}

void
SimpleProxyHandler::send_to(string host, const http::Request &req, http::Reply &reply)
{
	http::Request storage_fw(req);
	storage_fw.set_host(host);
	storage_fw.send(reply);
}
