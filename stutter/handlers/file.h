#ifndef HANDLER_FILE_H
#define HANDLER_FILE_H

#include <stutter/handlers/base.h>
#include <string>

class FileHandler : public virtual BaseHandler {
public:
    FileHandler();
    virtual ~FileHandler();
    virtual void handle(http::Connection &cx, http::Request &req, http::Reply &reply);

};

#endif // HANDLER_FILE_H
