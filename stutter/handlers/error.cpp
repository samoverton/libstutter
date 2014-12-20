#include <stutter/handlers/error.h>

using namespace std;

ErrorHandler::ErrorHandler(int status, string msg)
    : BaseHandler()
    , m_status(status)
    , m_msg(msg)
{
}

ErrorHandler::~ErrorHandler()
{
}

void
ErrorHandler::handle(http::Connection &cx, http::Request &req, http::Reply &reply)
{
    (void)req;
    reply.set_status(m_status, m_msg);
}
