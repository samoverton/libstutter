#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include "../http_connection.h"
#include "../http_request.h"
#include "../http_reply.h"

class BaseHandler {
public:
	BaseHandler(HttpConnection &cx);
	virtual void handle(const HttpRequest &req, HttpReply &reply) = 0;

protected:
	HttpConnection &connection();

private:
	HttpConnection &m_cx;
};

#endif // HANDLER_BASE_H
