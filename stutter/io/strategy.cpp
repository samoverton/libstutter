#include <stutter/io/strategy.h>
#include <stutter/http/body.h>
#include <stutter/log.h>

using namespace std;

IOStrategy::~IOStrategy()
{
}

bool
IOStrategy::send_raw(int fd, const char *data, size_t sz)
{
	size_t done = 0;
	while (done != sz) {
		int ret = safe_write(fd, data + done, sz - done);
		if (ret <= 0) {
			Log::get(Log::DEBUG) << "Failed to send " << (sz-done)
								 << " bytes to fd " << fd << endl;
			return false;
		}
		done += ret;
	}
	return true;
}

bool
IOStrategy::send_buffered_body(int out_fd, const http::Body &body)
{
	int in_fd = body.disk_fd();
	if (in_fd <= 0) // no data
		return true;

	size_t remain = body.size_on_disk();
	off_t offset = 0;
	while (remain > 0) {
		int sent = safe_sendfile(out_fd, in_fd, &offset, remain);
		if (sent <= 0) {
			Log::get(Log::DEBUG) << "Failed to sendfile() " << remain << " bytes" << endl;
			return false;
		}
		offset += sent;
		remain -= sent;
	}
	return true;
}
