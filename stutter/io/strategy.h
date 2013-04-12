#ifndef IO_STRATEGY_H
#define IO_STRATEGY_H

#include <cstring>
#if __APPLE__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#else
#include <sys/sendfile.h>
#endif

namespace http {
class Body;
}

class IOStrategy {
public:
	virtual ~IOStrategy();

	// read(2) and write(2) wrappers
	virtual int safe_read (int fd, char *p, size_t sz) = 0;
	virtual int safe_write(int fd, const char *p, size_t sz) = 0;

	// sendfile wrapper
	virtual int safe_sendfile(int out_fd, int in_fd, off_t *offset, size_t count) = 0;

	// send a whole block of data using `safe_write'
	bool send_raw (int fd, const char *data, size_t sz);

	bool send_buffered_body(int out_fd, const http::Body &body);
};

#endif // IO_STRATEGY_H
