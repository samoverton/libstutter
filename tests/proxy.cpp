#include <stutter/server.h>
#include <stutter/log.h>
#include <stutter/handlers/base.h>
#include <stutter/http/proxy.h>

#include <iostream>
#include <unistd.h>

using namespace std;

class PongHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		reply.add_body("pong", 4);
	}
};

class ProxyHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		// create new request
		http::Request fw(req);
		fw.set_url("/ping");

		// proxy
		http::Proxy px(cx, fw);
		px.send("localhost", 8888, reply);
	}
};

class DoubleProxyHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		// create new request
		http::Request fw(req);
		fw.set_url("/ping");

		// proxy
		http::Proxy px(cx, fw);
		px.send("localhost", 8888, reply); // once...
		px.send("localhost", 8888, reply); // twice. The response is now "pongpong"
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
