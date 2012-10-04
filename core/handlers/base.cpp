#include "base.h"

HandlerBase::HandlerBase(HttpConnection &cx)
	: m_cx(cx)
{
}

HttpConnection &
HandlerBase::connection()
{
	return m_cx;
}
