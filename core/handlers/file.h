#ifndef HANDLER_FILE_H
#define HANDLER_FILE_H

#include "base.h"
#include <string>

class FileHandler : public virtual BaseHandler {
public:
	FileHandler(http::Connection &cx);
	virtual ~FileHandler();
	virtual void handle(const http::Request &req, http::Reply &reply);

};

#endif // HANDLER_FILE_H
