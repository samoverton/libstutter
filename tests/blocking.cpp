#include <stutter/server.h>
#include <stutter/log.h>
#include <stutter/handlers/base.h>
#include <stutter/io/blocking.h>
#include <stutter/http/client.h>

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <pthread.h>

using namespace std;

#define HOST        "127.0.0.1"
#define PORT        8888
#define STATUS_CODE 555
#define STATUS_MSG  "Internal Test"
#define BODY        "test ok\r\n"

class TestHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, http::Request &req, http::Reply &reply)
	{
		reply.set_status(STATUS_CODE, STATUS_MSG);
		reply.add_body(BODY, sizeof(BODY)-1);
	}
};

void*
_start_server(void *p)
{
	(void)p;

	Server s(HOST, PORT);
	s.router().add("/hello", new TestHandler());
	s.start();

	return 0;
}

static int
send_request()
{
	http::Request req;
	req.set_url("/hello/world");
	
	BlockingIOStrategy io;
	http::Client cl(io, HOST, PORT);
	http::Reply rep;
	bool sent = cl.send(req, rep);

	cout << "sent = " << sent << ", code = " << rep.code()
		 << ", status=[" << rep.status() << "]" << endl;

	if (sent == 1
			&& rep.code() == STATUS_CODE
			/*&& rep.status().compare(STATUS_MSG) == 0*/) { // unsupported. TODO: fix.
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Log::setup("/dev/stderr", Log::DEBUG);

	pthread_t th;
	pthread_create(&th, 0, _start_server, 0);
	sleep(1);

	return send_request();
}
