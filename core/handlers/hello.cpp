#include "hello.h"

using namespace std;

HelloHandler::HelloHandler(HttpConnection &cx)
	: BaseHandler(cx)
{
}

void
HelloHandler::handle(const HttpRequest &req, HttpReply &reply)
{
	/*
	   send_to("storagevm", req, reply);

	   if (reply.code() != 200 && reply.code() != 204)
		   return;

	   send_to("jbirdvm", req, reply);
	*/

	reply.add_body("hello\n", 6);
}

void
HelloHandler::send_to(string host, const HttpRequest &req, HttpReply &reply)
{
	/*
	HttpRequest storage_fw(req);
	storage_fw.set_host(host);
	storage_fw.send(storage_reply);
	*/
}
