#include <stutter/server.h>
#include <stutter/log.h>
#include <stutter/handlers/base.h>
#include <stutter/http/client.h>

#include <iostream>
#include <unistd.h>

using namespace std;

class PongHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, http::Request &req, http::Reply &reply)
	{
		reply.add_body("pong\n", 5);
	}
};

class ProxyHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, http::Request &req, http::Reply &reply)
	{
		// create new request
		http::Request fw(req);
		fw.set_url("/ping");

		// proxy
		string target("localhost");
		http::Client px(cx, target, 8888);
		px.send(fw, reply);
	}
};

class DoubleProxyHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, http::Request &req, http::Reply &reply)
	{
		// create new request
		http::Request fw(req);
		fw.set_url("/ping");

		// proxy
		string target("localhost");
		http::Client px(cx, target, 8888);
		px.send(fw, reply); // once...
		px.send(fw, reply); // twice. The response is now "pong\npong\n"
	}
};

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Server s("127.0.0.1", 8888);
	// Log::setup("/dev/stderr", Log::DEBUG);
	s.router().add("/ping",  new PongHandler());
	s.router().add("/proxy", new ProxyHandler());
	s.router().add("/double", new DoubleProxyHandler());

	s.start();

	return 0;
}
