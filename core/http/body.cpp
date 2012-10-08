#include "body.h"
#include "connection.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 8192
#define TMP_FILE_TEMPLATE "/tmp/body-XXXXXX"

using http::Body;
using http::Connection;
using namespace std;

Body::Body()
	: m_fd(-1)
	, m_size(0)
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
	if ((m_fd = mkstemp(filename)) > 0) {
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

// send

bool
Body::send(Connection &cx)
{
	return send_from_memory(cx)
		&& send_from_disk(cx);
}

bool
Body::send_from_memory(Connection &cx) const
{
	size_t done = 0;
	while (done < m_data.size()) {
		int sent = cx.safe_write(&m_data[done], m_data.size() - done);
		if (sent <= 0) {
			// TODO: log
			return false;
		}
		done += sent;
	}
	return true;
}

bool
Body::send_from_disk(Connection &cx) const
{
	if (m_fd <= 0)
		return true;

	size_t remain = size() - m_data.size();
	off_t offset = 0;
	while (remain > 0) {
		int sent = cx.safe_write(m_fd, &offset, remain);
		if (sent <= 0) {
			// TODO: log
			return false;
		}
		offset += sent;
		remain -= sent;
	}

	return true;
}
