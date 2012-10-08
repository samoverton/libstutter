#ifndef HTTP_BODY_H
#define HTTP_BODY_H

#include <string>
#include <vector>

namespace http {
class Body;

class BodyIterator {
public:
	BodyIterator(Body &b);
	
};

class Body {
public:
	Body();
	ssize_t add(const char *p, size_t sz);
	void clear();
	size_t size() const;

	typedef BodyIterator iterator;
	iterator begin() const;
	iterator end() const;

private:
	ssize_t buffer_in_memory(const char *p, size_t sz);
	ssize_t buffer_on_disk(const char *p, size_t sz);
	bool create_file();

private:
	std::vector<char> m_data;
	std::string m_filename;
	int m_fd;
	size_t m_size;

friend class BodyIterator;
};

}

#endif // HTTP_BODY_H
