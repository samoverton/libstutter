#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>
#include <map>
#include <vector>

namespace http {
class Message {

public:
	void add_header(std::string key, std::string val);
	void add_header(std::string key, int val);
	void add_body(const char *p, size_t sz);
	virtual void reset();

	typedef std::vector<char>::const_iterator iterator;
	iterator begin() const;
	iterator end() const;

protected:
	std::map<std::string, std::string> m_headers;
	std::vector<char> m_data;
	std::vector<char> m_body;
};
}

#endif // HTTP_MESSAGE_H
