#include "hello.h"

using namespace std;

HelloHandler::HelloHandler(http::Connection &cx)
	: BaseHandler(cx)
{
}

void
HelloHandler::handle(const http::Request &req, http::Reply &reply)
{
	(void)req;
	char message[] = "hello, world\n";
	reply.add_body(message, sizeof(message)-1);
}
