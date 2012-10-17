#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <map>
#include <vector>
#include <string>

class BaseHandler;

class Dispatcher {
public:
	bool add(std::string prefix, BaseHandler *b);
	BaseHandler *get(std::string prefix) const;

private:
	typedef std::vector<std::pair<std::string, BaseHandler*> > handler_container;
	handler_container m_handlers;
};

#endif // DISPATCHER_H
