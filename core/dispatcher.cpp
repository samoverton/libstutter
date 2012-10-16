#include "dispatcher.h"
#include <handlers/base.h>

using namespace std;

bool
Dispatcher::add(string prefix, BaseHandler *b)
{
	m_handlers.push_back(make_pair(prefix, b));
	return true;
}


BaseHandler *
Dispatcher::get(string prefix) const
{
	handler_container::const_iterator it;
	for (it = m_handlers.begin(); it != m_handlers.end(); it++) {
		if (it->first.compare(0, prefix.size(),
					prefix.c_str(), prefix.size()) == 0)
			return it->second;
	}
	return 0;
}

