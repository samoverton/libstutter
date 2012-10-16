#include "file.h"

using namespace std;

FileHandler::FileHandler()
	: BaseHandler()
{
}

FileHandler::~FileHandler()
{
}

void
FileHandler::handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
{
	(void)req;
	// char message[] = "this should be served with sendfile\n";
	// reply.add_body(message, sizeof(message)-1);

	reply.set_file("/tmp/reply.txt");
}
