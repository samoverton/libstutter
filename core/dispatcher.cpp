#include "dispatcher.h"
#include <handlers/base.h>

#include <iostream>

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

		if (it->first.size() <= prefix.size() &&
				it->first.compare(0, it->first.size(),
					prefix.c_str(), it->first.size()) == 0)
			return it->second;
	}
	cout << "return 0" << endl;
	return 0;
}

