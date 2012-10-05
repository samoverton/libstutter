#include "pool.h"

using namespace std;

bool
SocketPool::get(int &fd)
{
	if (m_avail.empty())
		return false;

	fd = *m_avail.begin();
	m_avail.erase(fd);
	m_taken.insert(fd);

	return true;
}

bool
SocketPool::put(int fd)
{
	if (m_taken.erase(fd) == 0)
		return false;

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

////////////////////////////////////////////////////////////////////////////////

SocketPool &
PoolManager::get_pool(string host)
{
	// find or insert host
	map<string, SocketPool>::iterator it = m_pools.find(host);
	if (it == m_pools.end())
		it = m_pools.insert(make_pair(host, SocketPool())).first;

	return it->second;
}

