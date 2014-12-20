#include <stutter/handlers/file.h>

using namespace std;

FileHandler::FileHandler()
    : BaseHandler()
{
}

FileHandler::~FileHandler()
{
}

void
FileHandler::handle(http::Connection &cx, http::Request &req, http::Reply &reply)
{
    (void)req;

    // example:
    // reply.set_file("/tmp/reply.txt");
}
