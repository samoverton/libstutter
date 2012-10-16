#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <map>
#include <string>

class BaseHandler;

class Dispatcher {
public:
	bool add(std::string prefix, BaseHandler *b);

	BaseHandler *get(std::string prefix) const;

private:
	std::map<std::string, BaseHandler*> m_handlers;
};

#endif // DISPATCHER_H
