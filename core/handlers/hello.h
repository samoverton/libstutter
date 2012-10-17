#ifndef HANDLER_HELLO_H
#define HANDLER_HELLO_H

#include <handlers/base.h>
#include <string>

class HelloHandler : public virtual BaseHandler {
public:
	HelloHandler();
	virtual ~HelloHandler();
	virtual void handle(http::Connection &cx,
			const http::Request &req, http::Reply &reply);

};

#endif // HANDLER_HELLO_H
