#ifndef POOL_H
#define POOL_H

#include <set>
#include <map>
#include <string>

class SocketPool {
public:
	SocketPool(std::string host);

	bool get(int &fd);
	bool put(int fd);
	bool del(int fd);

private:
	bool connect(int &fd);

private:
	std::string m_host;
	std::set<int> m_avail;
	std::set<int> m_taken;
};

class PoolManager {
public:
	SocketPool &get_pool(std::string host);

private:
	std::map<std::string, SocketPool> m_pools;

};

#endif // POOL_H
