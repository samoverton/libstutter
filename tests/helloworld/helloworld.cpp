#include <server.h>
#include <log.h>
#include <handlers/base.h>

#include <iostream>

using namespace std;

class HelloWorldHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		reply.add_body("hi\r\n", 4);
	}
};

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Server s("127.0.0.1", 8888);
	s.router().add("/hello", new HelloWorldHandler());
	//Log::setup("/tmp/stutter.log", Log::INFO);

	s.start();

	return 0;
}
