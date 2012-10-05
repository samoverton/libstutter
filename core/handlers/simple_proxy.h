#ifndef HANDLER_SIMPLE_PROXY_H
#define HANDLER_SIMPLE_PROXY_H

#include "base.h"
#include <string>

class SimpleProxyHandler : public virtual BaseHandler {
public:
	SimpleProxyHandler(HttpConnection &cx);
	virtual void handle(const http::Request &req, HttpReply &reply);

private:
	void send_to(std::string host, const http::Request &req, HttpReply &reply);

};

#endif // HANDLER_HELLO_H
