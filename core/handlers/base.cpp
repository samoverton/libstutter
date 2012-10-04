#include "base.h"

BaseHandler::BaseHandler(HttpConnection &cx)
	: m_cx(cx)
{
}

HttpConnection &
BaseHandler::connection()
{
	return m_cx;
}
