#include "dispatcher.h"
#include <handlers/base.h>

using namespace std;

bool
Dispatcher::add(string prefix, BaseHandler *b)
{
	if (get(prefix) != 0)
		return false;

	m_handlers.insert(make_pair(prefix, b));
	return true;
}


BaseHandler *
Dispatcher::get(string prefix) const
{
	map<string, BaseHandler*>::const_iterator it = m_handlers.find(prefix);
	if (it != m_handlers.end())
		return 0;

	return it->second;
}

