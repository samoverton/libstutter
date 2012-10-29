#include <stutter/io/blocking.h>

#include <unistd.h>

BlockingIOStrategy::BlockingIOStrategy()
{
}

BlockingIOStrategy::~BlockingIOStrategy()
{
}

int
BlockingIOStrategy::safe_read (int fd, char *p, size_t sz)
{
	return ::read(fd, p, sz);
}

int
BlockingIOStrategy::safe_write(int fd, const char *p, size_t sz)
{
	return ::write(fd, p, sz);
}


int
BlockingIOStrategy::safe_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
	return (int)::sendfile(out_fd, in_fd, offset, count);
}

