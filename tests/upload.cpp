#include <server.h>
#include <log.h>
#include <handlers/base.h>

#include <iostream>
#include <sstream>

#include "framework.h"

using namespace std;

class UploadHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		stringstream ss;
		ss << "sz=" << req.body().size();

		string body = ss.str();
		reply.add_body(body.c_str(), body.size());
	}
};

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Server s("127.0.0.1", 8888);
	s.router().add("/upload", new UploadHandler());
	s.router().add("/quit", new TestQuitHandler());

	s.start();

	return 0;
}
