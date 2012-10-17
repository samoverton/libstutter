#ifndef HANDLER_SIMPLE_PROXY_H
#define HANDLER_SIMPLE_PROXY_H

#include "base.h"
#include <string>

namespace http {
class Connection;
}

class SimpleProxyHandler : public virtual BaseHandler {
public:
	SimpleProxyHandler();
	virtual void handle(http::Connection &cx,
			const http::Request &req, http::Reply &reply);

};

#endif // HANDLER_HELLO_H
