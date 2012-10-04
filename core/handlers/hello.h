#ifndef HANDLER_HELLO_H
#define HANDLER_HELLO_H

#include "base.h"
#include <string>

class HelloHandler : public virtual BaseHandler {
public:
	HelloHandler(HttpConnection &cx);
	virtual void handle(const HttpRequest &req, HttpReply &reply);

private:
	void send_to(std::string host, const HttpRequest &req, HttpReply &reply);

};

#endif // HANDLER_HELLO_H
