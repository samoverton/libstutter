#include <handlers/error.h>

using namespace std;

ErrorHandler::ErrorHandler(http::Connection &cx)
	: BaseHandler(cx)
{
}

ErrorHandler::~ErrorHandler()
{
}

void
ErrorHandler::handle(const http::Request &req, http::Reply &reply)
{
	(void)req;
	char message[] = "hello, world\n";
	reply.add_body(message, sizeof(message)-1);
}
