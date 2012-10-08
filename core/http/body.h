#ifndef HTTP_BODY_H
#define HTTP_BODY_H

#include <string>
#include <vector>

namespace http {

class Connection;

class Body {
public:
	Body();
	~Body();
	ssize_t add(const char *p, size_t sz);
	void clear();
	size_t size() const;

	bool send(Connection &cx);

private:
	ssize_t buffer_in_memory(const char *p, size_t sz);
	ssize_t buffer_on_disk(const char *p, size_t sz);
	bool create_file();

	bool send_from_memory(Connection &cx) const;
	bool send_from_disk(Connection &cx) const;

private:
	std::vector<char> m_data;
	std::string m_filename;
	int m_fd;
	size_t m_size;

};

}

#endif // HTTP_BODY_H
