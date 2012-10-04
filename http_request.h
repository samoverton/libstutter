#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>

class HttpRequest {
public:
	const std::string &url() const;
	void add_header(std::string &key, std::string &val);
	void add_url_fragment(const char *at, size_t sz);

private:
	std::string m_url;
	std::map<std::string, std::string> m_headers;

friend class HttpClient;
};

#endif // HTTP_REQUEST_H
