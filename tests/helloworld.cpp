#include <server.h>
#include <log.h>
#include <handlers/base.h>

#include <iostream>
#include <unistd.h>

#include "framework.h"

using namespace std;

class HelloWorldHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		reply.add_body("hello\r\n", 7);
	}
};

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Server s("127.0.0.1", 8888);
	s.router().add("/hello", new HelloWorldHandler());
	s.router().add("/quit", new TestQuitHandler());

	s.start();

	return 0;
}
