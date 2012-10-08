#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "message.h"

namespace http {
class Reply;
class Connection;

class Request : public virtual Message {
public:
	Request(Connection &connection);
	Request(const Request &request);
	const std::string &url() const;
	void add_url_fragment(const char *at, size_t sz);
	void set_host(std::string host);

	typedef enum {GET, POST, PUT, HEAD, DELETE} Verb;
	void set_verb(Verb v);
	Verb verb() const;
	std::string verb_str() const;

	typedef enum {NOT_EXECUTED = -1, SUCCESS, SOCKET_ERROR,
		DNS_ERROR, CONNECTION_ERROR, WRITE_ERROR} Error;
	Error send(Reply &reply);
	void error(Error e);

	// from message
	virtual void prepare();
	virtual void reset();

private:
	bool connect();
	bool send();
	bool read_reply(Reply &reply);

protected:
	virtual bool send_headers();
	virtual bool send_body();

private:
	int m_fd;
	Verb m_verb;
	std::string m_url;
	std::string m_host;

	Connection &m_connection;
	Error m_error;

friend class Connection;
friend void _done(void*);
};

}
#endif // HTTP_REQUEST_H
