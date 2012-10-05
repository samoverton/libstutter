#ifndef HANDLER_HELLO_H
#define HANDLER_HELLO_H

#include "base.h"
#include <string>

class HelloHandler : public virtual BaseHandler {
public:
	HelloHandler(http::Connection &cx);
	virtual void handle(const http::Request &req, http::Reply &reply);

};

#endif // HANDLER_HELLO_H
