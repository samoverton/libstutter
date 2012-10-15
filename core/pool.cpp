#include "pool.h"
#include "log.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define MAX_POOL_SIZE  1024

using namespace std;

SocketPool::SocketPool(std::string host)
	: m_host(host)
{
	
}

bool
SocketPool::get(int &fd)
{
	if (m_avail.empty()) {
		if (m_taken.size() >= MAX_POOL_SIZE)
			return false;

		return connect(fd);
	}

	fd = *m_avail.begin();
	m_avail.erase(fd);
	m_taken.insert(fd);

	return true;
}

bool
SocketPool::put(int fd)
{
	if (m_taken.erase(fd) == 0) {
		return false;
	}

	m_avail.insert(fd);

	return true;
}

bool
SocketPool::del(int fd)
{
	m_taken.erase(fd);
	m_avail.erase(fd);

	return true;
}

bool
SocketPool::connect(int &out_fd)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) {
		return false;
	}

	// set socket as non-blocking.
	int ret = fcntl(fd, F_SETFD, O_NONBLOCK);
	if (0 != ret) {
		Log::get(Log::ERROR) << "Could not set pool fd as nonblocking" << endl;
		return false;
	}

	struct addrinfo *info = 0;
	ret = getaddrinfo(m_host.c_str(), "8080", 0, &info);
	if (ret < 0) {
		Log::get(Log::ERROR) << "Could not resolve host [" << m_host << "]" << endl;
		return false;
	}

	bool success = false;
	struct addrinfo *ai;
	for (ai = info; ai; ai = ai->ai_next) {
		struct sockaddr_in *sin = (struct sockaddr_in*)ai->ai_addr;
		int ret = ::connect(fd, (const struct sockaddr*)sin,
				sizeof(struct sockaddr_in));
		if (ret == 0) {
			out_fd = fd;
			m_taken.insert(fd);
			success = true;
		}
		break;
	}
	freeaddrinfo(info);

	if (!success)
		Log::get(Log::ERROR) << "Could not connect to host [" << m_host << "]" << endl;

	return success;
}

////////////////////////////////////////////////////////////////////////////////

SocketPool &
PoolManager::get_pool(string host)
{
	// find or insert host
	map<string, SocketPool>::iterator it = m_pools.find(host);
	if (it == m_pools.end())
		it = m_pools.insert(make_pair(host, SocketPool(host))).first;

	return it->second;
}

