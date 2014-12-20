#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include <stutter/http/connection.h>
#include <stutter/http/request.h>
#include <stutter/http/reply.h>
#include <stutter/pool.h>

class BaseHandler {
public:
    BaseHandler();
    virtual ~BaseHandler();
    virtual void handle(http::Connection &cx,
            http::Request &req, http::Reply &reply) = 0;
};

#endif // HANDLER_BASE_H
