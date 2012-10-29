#ifndef HTTP_BODY_H
#define HTTP_BODY_H

#include <string>
#include <vector>

class IOStrategy;

namespace http {

class Body {
public:
	Body();
	Body(const Body &b);
	~Body();
	ssize_t add(const char *p, size_t sz);
	void clear();
	size_t size() const;

	bool set_file(const std::string &name);

	typedef std::vector<char>::const_iterator iterator;
	const iterator buffer_begin() const;
	const iterator buffer_end() const;

	// used with sendfile()
	int disk_fd() const;
	size_t size_on_disk() const;

private:
	ssize_t buffer_in_memory(const char *p, size_t sz);
	ssize_t buffer_on_disk(const char *p, size_t sz);
	bool create_file();

private:
	std::vector<char> m_data;
	std::string m_filename;
	int m_fd;
	size_t m_size;
	bool m_unlink;
};

}

#endif // HTTP_BODY_H
