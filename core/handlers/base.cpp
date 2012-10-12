#include "base.h"
#include "../server.h"

BaseHandler::BaseHandler(http::Connection &cx)
	: m_poolmgr(cx.server().pool_manager())
	, m_cx(cx)
{
}

BaseHandler::~BaseHandler()
{
}

http::Connection &
BaseHandler::connection()
{
	return m_cx;
}
