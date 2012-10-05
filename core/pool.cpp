#include "pool.h"

bool
Pool::get(int &i)
{
	if (m_avail.empty())
		return false;

	i = *m_avail.begin();
	m_avail.erase(i);
	m_taken.insert(i);

	return true;
}

bool
Pool::put(int i)
{
	if (m_taken.erase(i) == 0)
		return false;

	m_avail.insert(i);

	return true;
}

