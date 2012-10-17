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

	reply.set_file("/tmp/reply.txt");
}
