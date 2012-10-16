#include <server.h>
#include <log.h>
#include <handlers/base.h>

#include <iostream>

using namespace std;

class HelloWorldHandler : public BaseHandler {
public:
	virtual void handle(const http::Request &req, http::Reply &reply)
	{
		cout << "HelloWorldHandler" << endl;
	}
};

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Server s("127.0.0.1", 8888);
	//Log::setup("/tmp/stutter.log", Log::INFO);

	s.start();

	return 0;
}
