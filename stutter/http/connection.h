#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <stutter/coroutine.h>
#include <stutter/http/parser.h>
#include <stutter/http/request.h>
#include <stutter/http/reply.h>
#include <stutter/io/yielding.h>

#include <event.h>
#include <string>
#include <map>

class Server;

namespace http {

class Connection : public YieldingIOStrategy {
public:

    Connection(Server &server, int fd);
    virtual ~Connection();
    virtual int exec();

    int fd() const;
    Server &server();

    bool process(); // callback

private:
    bool send(Reply &r);
    bool send_headers(Reply &r);
    bool send_100_continue();
    void process_error();

    typedef enum {
        ST_READY_FOR_DATA,
        ST_NEED_100_CONTINUE,
        ST_IO_ERROR,
        ST_PARSE_ERROR,
        ST_SHOULD_CLOSE
    } State;

private:
    Server &m_server;
    int m_fd;

    http::Request m_request;
    http::Reply   m_reply;
    http::Parser  m_parser;

    State         m_state;

friend void _process(void*);
};
}

#endif // HTTP_CONNECTION_H
