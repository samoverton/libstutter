#ifndef HTTP_PROXY_H
#define HTTP_PROXY_H

#include <string>

namespace http {

class Connection;
class Request;
class Reply;

class Proxy
{
public:
	Proxy(Connection &cx, const Request &req);

	bool send(const std::string host, Reply &reply);

private:
	Connection &m_connection;
	const Request &m_request;
};
}

#endif // HTTP_PROXY_H
