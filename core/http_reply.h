#ifndef HTTP_REPLY_H
#define HTTP_REPLY_H

#include <string>
#include <map>
#include <vector>

class HttpClient;

class HttpReply {

public:
	HttpReply(HttpClient &client);

	void set_status(short code, std::string status);
	void add_header(std::string key, std::string val);
	void add_header(std::string key, int val);
	void add_body(const char *p, size_t sz);

	void reset();
	void prepare();

	typedef std::vector<char>::const_iterator iterator;
	iterator begin() const;
	iterator end() const;

private:
	std::map<std::string, std::string> m_headers;
	short m_code;
	std::string m_status;
	std::vector<char> m_data;
	std::vector<char> m_body;

	HttpClient &m_client;
};

#endif // HTTP_REPLY_H
