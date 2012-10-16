#ifndef HANDLER_ERROR_H
#define HANDLER_ERROR_H

#include <handlers/error.h>
#include <string>

class ErrorHandler : public virtual BaseHandler {
public:
	ErrorHandler(http::Connection &cx);
	virtual ~ErrorHandler();
	virtual void handle(const http::Request &req, http::Reply &reply);

};

#endif // HANDLER_ERROR_H
