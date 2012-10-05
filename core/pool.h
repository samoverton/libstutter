#ifndef POOL_H
#define POOL_H

#include <set>

class Pool {
public:
	bool get(int &i);
	bool put(int i);

private:

	std::set<int> m_avail;
	std::set<int> m_taken;

};

#endif // POOL_H
