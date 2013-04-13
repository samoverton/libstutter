#include <stutter/io/yielding.h>

#include <unistd.h>

YieldingIOStrategy::YieldingIOStrategy()
{
}

YieldingIOStrategy::~YieldingIOStrategy()
{
}

int
YieldingIOStrategy::watched_fd() const
{
	return m_watched_fd;
}

void
YieldingIOStrategy::watch_fd(int fd)
{
	m_watched_fd = fd;
}

struct event *
YieldingIOStrategy::event()
{
	return &m_ev;
}

int
YieldingIOStrategy::safe_read (int fd, char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)READ);

	return ::read(fd, p, sz);
}

int
YieldingIOStrategy::safe_write(int fd, const char *p, size_t sz)
{
	watch_fd(fd);
	yield((int)WRITE);

	return ::write(fd, p, sz);
}

int
YieldingIOStrategy::safe_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
	watch_fd(out_fd);
	yield((int)WRITE);

#ifdef __APPLE__
	off_t len = (off_t)count;
	if (::sendfile(out_fd, in_fd, *offset, &len, 0, 0) == 0) {
		return (int)len;
	}
	return -1;
#else
	return (int)::sendfile(out_fd, in_fd, offset, count);
#endif
}

