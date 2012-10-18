#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include <core/http/connection.h>
#include <core/http/request.h>
#include <core/http/reply.h>
#include <core/pool.h>

class BaseHandler {
public:
	BaseHandler();
	virtual ~BaseHandler();
	virtual void handle(http::Connection &cx,
			const http::Request &req, http::Reply &reply) = 0;
};

#endif // HANDLER_BASE_H
