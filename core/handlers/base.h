#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include "../http/connection.h"
#include "../http/request.h"
#include "../http/reply.h"
#include "../pool.h"

class BaseHandler {
public:
	BaseHandler();
	virtual ~BaseHandler();
	virtual void handle(http::Connection &cx,
			const http::Request &req, http::Reply &reply) = 0;
};

#endif // HANDLER_BASE_H
