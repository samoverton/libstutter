#include "body.h"

#include <iostream>

#define MAX_BUFFER_SIZE 4096
#define TMP_FILE_TEMPLATE "/tmp/body-XXXXXX"

using http::Body;
using namespace std;

Body::Body()
	: m_fd(-1)
	, m_size(0)
{
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
	m_filename.clear();
	if (m_fd > 0) {
		::close(m_fd);
		m_fd = -1;
	}
	m_size = 0;
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
	cout << "buffered " << insert << " bytes in memory" << endl;
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
	cout << "buffered " << ret << " bytes on disk" << endl;
	return ret;
}
