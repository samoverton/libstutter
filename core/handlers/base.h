#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include "../http/connection.h"
#include "../http/request.h"
#include "../http/reply.h"
#include "../pool.h"

class BaseHandler {
public:
	BaseHandler(http::Connection &cx);
	virtual void handle(const http::Request &req, http::Reply &reply) = 0;

protected:
	http::Connection &connection();
	PoolManager &m_poolmgr;

private:
	http::Connection &m_cx;
};

#endif // HANDLER_BASE_H
