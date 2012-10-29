#include <stutter/http/body.h>
#include <stutter/io/strategy.h>
#include <stutter/log.h>

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>

#define MAX_BUFFER_SIZE 8192
#define TMP_FILE_TEMPLATE "/tmp/body-XXXXXX"

using http::Body;
using namespace std;

Body::Body()
	: m_fd(-1)
	, m_size(0)
	, m_unlink(false)
{
}

Body::Body(const Body &b)
	: m_data(b.m_data)
	, m_filename(b.m_filename)
	, m_fd(dup(b.m_fd))
	, m_size(b.m_size)
	, m_unlink(false)
{
}

Body::~Body()
{
	clear();
}

ssize_t
Body::add(const char *p, size_t sz)
{
	if (m_data.size() == MAX_BUFFER_SIZE)
		return buffer_on_disk(p, sz);

	return buffer_in_memory(p, sz);
}

void
Body::clear()
{
	m_data.clear();
	if (m_fd > 0) {
		::close(m_fd);
		m_fd = -1;
	}
	if (m_unlink)
		unlink(m_filename.c_str());
	m_filename.clear();
	m_size = 0;
	m_unlink = false;
}

size_t
Body::size() const
{
	return m_size;
}

ssize_t
Body::buffer_in_memory(const char *p, size_t sz)
{
	size_t avail = MAX_BUFFER_SIZE - m_data.size();
	size_t insert = (sz > avail ? avail : sz);

	m_data.insert(m_data.end(), p, p + insert);
	m_size += insert;
	return insert;
}

bool
Body::create_file()
{
	if (m_fd > 0)
		return true;

	char filename[] = TMP_FILE_TEMPLATE;
	if ((m_fd = ::mkstemp(filename)) > 0) {
		m_filename = filename;
		m_unlink = true;
		return true;
	}
	return false;
}

ssize_t
Body::buffer_on_disk(const char *p, size_t sz)
{
	if (!create_file())
		return -1;

	int ret = write(m_fd, p, sz);
	if (ret > 0)
		m_size += ret;
	return ret;
}

bool
Body::set_file(const string &name)
{
	int fd;
	if ((fd = ::open(name.c_str(), O_RDONLY)) < 0)
		return false;

	clear();
	m_fd = fd;
	m_filename = name;

	struct stat st;
	if (fstat(m_fd, &st) == 0)
		m_size = st.st_size;

	return true;
}

const Body::iterator
Body::buffer_begin() const
{
	return m_data.begin();
}

const Body::iterator
Body::buffer_end() const
{
	return m_data.end();
}

int
Body::disk_fd() const
{
	return m_fd;
}

size_t
Body::size_on_disk() const
{
	return size() - m_data.size();
}
