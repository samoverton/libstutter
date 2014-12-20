#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>

class IOStrategy;

namespace http {

class Request;
class Reply;
class Parser;

class Client
{
public:
    Client(IOStrategy &io, const std::string host, short port);

    bool send(Request &req, Reply &reply);

    typedef enum {NOT_EXECUTED = -1, SUCCESS, SOCKET_ERROR,
        DNS_ERROR, CONNECTION_ERROR, WRITE_ERROR, READ_ERROR} Error;
private:

    // steps
    bool connect();
    bool send_headers(Request &r);
    bool wait_for_100();
    bool send_body(Request &r);

    bool failure(Error e);
    void error(Error e);
    bool read_reply(Parser &parser);
    void release_socket();
    bool on_msg_complete();

private:
    int m_fd;
    IOStrategy &m_io;

    Error m_error;
    bool  m_done;

    // target
    const std::string m_host;
    short m_port;

friend bool _done(void*);
};
}

#endif // HTTP_CLIENT_H
