#ifndef HANDLER_ERROR_H
#define HANDLER_ERROR_H

#include <stutter/handlers/base.h>
#include <string>

class ErrorHandler : public virtual BaseHandler {
public:
	ErrorHandler(int status, std::string msg);
	virtual ~ErrorHandler();
	virtual void handle(http::Connection &cx,
			const http::Request &req, http::Reply &reply);

private:
	int m_status;
	std::string m_msg;
};

#endif // HANDLER_ERROR_H
