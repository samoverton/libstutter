#include "base.h"

BaseHandler::BaseHandler(http::Connection &cx)
	: m_cx(cx)
{
}

http::Connection &
BaseHandler::connection()
{
	return m_cx;
}
