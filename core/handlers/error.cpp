#include <handlers/error.h>

using namespace std;

ErrorHandler::ErrorHandler()
	: BaseHandler()
{
}

ErrorHandler::~ErrorHandler()
{
}

void
ErrorHandler::handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
{
	(void)req;
	char message[] = "hello, world\n";
	reply.add_body(message, sizeof(message)-1);
}
