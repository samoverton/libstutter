#include "hello.h"

using namespace std;

HelloHandler::HelloHandler(HttpConnection &cx)
	: BaseHandler(cx)
{
}

void
HelloHandler::handle(const HttpRequest &req, HttpReply &reply)
{
	char message[] = "hello, world\n";
	reply.add_body(message, sizeof(message)-1);
}
