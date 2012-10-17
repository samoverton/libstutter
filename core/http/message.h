#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>
#include <map>
#include <vector>

#include "body.h"

namespace http {
class Connection;

class Message {
public:
	Message(Connection &cx);
	Message(const Message &msg);
	virtual ~Message();
	void add_header(std::string key, std::string val);
	void add_header(std::string key, int val);
	bool del_header(std::string key);
	void add_body(const char *p, size_t sz);

	std::string get_header(std::string key) const;
	const Body &body() const;

	// data iterator
	typedef std::vector<char>::const_iterator iterator;
	iterator begin() const;
	iterator end() const;

	virtual void reset();
	virtual void prepare() = 0;

	// public headers
	static const std::string Expect;
	static const std::string OneHundredContinue;

protected:
	std::map<std::string, std::string> m_headers;
	std::vector<char> m_data;
	Body m_body;

protected:
	Connection &m_connection;
};
}

#endif // HTTP_MESSAGE_H
