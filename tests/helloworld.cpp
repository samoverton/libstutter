#include <stutter/server.h>
#include <stutter/log.h>
#include <stutter/handlers/base.h>

#include <iostream>
#include <unistd.h>

using namespace std;

class HelloWorldHandler : public BaseHandler {
public:
    virtual void handle(http::Connection &cx, http::Request &req, http::Reply &reply)
    {
        reply.add_body("hello\r\n", 7);
    }
};

int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Log::setup("/dev/stderr", Log::TRACE);

    Server s("127.0.0.1", 8888);
    s.router().add("/hello", new HelloWorldHandler());

    s.start();

    return 0;
}
