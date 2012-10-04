#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>

class HttpConnection;
class HttpReply;

class HttpRequest {
public:
	HttpRequest(HttpConnection &connection);
	HttpRequest(const HttpRequest &request);
	const std::string &url() const;
	void add_header(std::string key, std::string val);
	void add_url_fragment(const char *at, size_t sz);
	void set_host(std::string host);

	void send(HttpReply &reply);

private:
	void connect();
	void prepare();
	void send();
	void read_reply(HttpReply &reply);

private:
	int m_fd;
	std::string m_url;
	std::string m_host;
	std::map<std::string, std::string> m_headers;

	HttpConnection &m_connection;
	std::string m_data;
	bool m_done;

friend class HttpConnection;
friend void _done(void*);
};

#endif // HTTP_REQUEST_H
