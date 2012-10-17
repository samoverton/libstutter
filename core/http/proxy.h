#ifndef HTTP_PROXY_H
#define HTTP_PROXY_H

#include <string>

namespace http {

class Connection;
class Request;
class Reply;
class Parser;

class Proxy
{
public:
	Proxy(Connection &cx, const Request &req);

	bool send(const std::string host, short port, Reply &reply);

	typedef enum {NOT_EXECUTED = -1, SUCCESS, SOCKET_ERROR,
		DNS_ERROR, CONNECTION_ERROR, WRITE_ERROR, READ_ERROR} Error;
private:

	// steps
	bool connect(const std::string &host, short port);
	bool send_headers(Request &r);
	bool wait_for_100();
	bool send_body(Request &r);

	bool failure(const std::string &host, short port, Error e);
	void error(Error e);
	bool read_reply(Parser &parser);
	void release_socket(const std::string &host, short port);
	void on_msg_complete();

private:
	int m_fd;
	Connection &m_connection;
	const Request &m_request;

	Error m_error;
	bool  m_done;

friend void _done(void*);
};
}

#endif // HTTP_PROXY_H
