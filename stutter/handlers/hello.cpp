#include <stutter/handlers/hello.h>

using namespace std;

HelloHandler::HelloHandler()
	: BaseHandler()
{
}

HelloHandler::~HelloHandler()
{
}

void
HelloHandler::handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
{
	(void)req;
	char message[] = "hello, world\n";
	reply.add_body(message, sizeof(message)-1);
}
