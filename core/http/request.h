#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "message.h"

namespace http {
class Reply;
class Connection;
class Parser;

class Request : public virtual Message {
public:
	Request(Connection &cx);
	Request(const Request &request);
	virtual ~Request();
	const std::string &url() const;
	void add_url_fragment(const char *at, size_t sz);
	void set_host(std::string host);

	typedef enum {GET, POST, PUT, HEAD, DELETE} Verb;
	void set_verb(Verb v);
	Verb verb() const;
	std::string verb_str() const;

	bool send_continue();

	typedef enum {NOT_EXECUTED = -1, SUCCESS, SOCKET_ERROR,
		DNS_ERROR, CONNECTION_ERROR, WRITE_ERROR} Error;
	Error send(Reply &reply);
	void error(Error e);

	// from message
	virtual void prepare();
	virtual void reset();

private:
	bool connect();
	bool send_data(Parser &parser, Reply &reply);
	bool read_reply(Parser &p, Reply &reply);
	void release_socket();

	bool send_raw(int fd, const char *data, size_t sz);

protected:
	virtual bool send_headers();
	bool send_body(Parser &parser, Reply &reply);

private:
	int m_fd;
	Verb m_verb;
	std::string m_url;
	std::string m_host;

	// upload
	bool m_require_100;

	Error m_error;

friend class Connection;
friend void _done(void*);
};

}
#endif // HTTP_REQUEST_H
