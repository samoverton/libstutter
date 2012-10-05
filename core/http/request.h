#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>

namespace http {
class Reply;
class Connection;

class Request {
public:
	Request(Connection &connection);
	Request(const Request &request);
	const std::string &url() const;
	void add_header(std::string key, std::string val);
	void add_url_fragment(const char *at, size_t sz);
	void set_host(std::string host);
	void reset();

	typedef enum {NOT_EXECUTED = -1, SUCCESS, SOCKET_ERROR,
		DNS_ERROR, CONNECTION_ERROR, WRITE_ERROR} Error;
	Error send(Reply &reply);
	void error(Error e);

private:
	bool connect();
	bool prepare();
	bool send();
	bool read_reply(Reply &reply);

private:
	int m_fd;
	std::string m_url;
	std::string m_host;
	std::map<std::string, std::string> m_headers;

	Connection &m_connection;
	std::string m_data;
	Error m_error;

friend class Connection;
friend void _done(void*);
};

}
#endif // HTTP_REQUEST_H
