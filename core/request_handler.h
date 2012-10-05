#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

namespace http {
	class Request;
}
class HttpReply;

class HandlerBase {
public:
	
	virtual void handle(const http::Request &req, HttpReply &reply) = 0;

};

#endif // HANDLER_BASE_H
