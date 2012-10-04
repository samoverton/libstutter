#include "simple_proxy.h"

using namespace std;

SimpleProxyHandler::SimpleProxyHandler(HttpConnection &cx)
	: BaseHandler(cx)
{
}

void
SimpleProxyHandler::handle(const HttpRequest &req, HttpReply &reply)
{
	HttpReply jbird_reply(connection());
	send_to("bea", req, jbird_reply);
	/*

	   if (reply.code() != 200 && reply.code() != 204)
		   return;

	   send_to("jbirdvm", req, reply);
	*/

	// reply.add_body("hello\n", 6);
}

void
SimpleProxyHandler::send_to(string host, const HttpRequest &req, HttpReply &reply)
{
	HttpRequest storage_fw(req);
	storage_fw.set_host(host);
	storage_fw.send(reply);
}
