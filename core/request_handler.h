#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

class HttpRequest;
class HttpReply;

class HandlerBase {
public:
	
	virtual void handle(const HttpRequest &req, HttpReply &reply) = 0;

};

#endif // HANDLER_BASE_H
