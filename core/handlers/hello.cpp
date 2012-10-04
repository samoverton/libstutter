#include "hello.h"

using namespace std;

void
HandlerHello::handle(const HttpRequest &req, HttpReply &reply)
{
	/*
	   send_to("storagevm", req, reply);

	   if (reply.code() != 200 && reply.code() != 204)
		   return;

	   send_to("jbirdvm", req, reply);
	*/

	reply.set_status(200, "OK");
	reply.add_body("hello\n", 6);
}

void
HandlerHello::send_to(string host, const HttpRequest &req, HttpReply &reply)
{
	/*
	HttpRequest storage_fw(req);
	storage_fw.set_host(host);
	storage_fw.send(storage_reply);
	*/
}
