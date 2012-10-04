#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include "../http_request.h"
#include "../http_reply.h"

class HandlerBase {
public:
	virtual void handle(const HttpRequest &req, HttpReply &reply) = 0;

};

#endif // HANDLER_BASE_H
