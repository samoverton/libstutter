#ifndef HTTP_REPLY_H
#define HTTP_REPLY_H

#include "message.h"

namespace http {
class Connection;

class Reply : public Message {

public:
	Reply(Connection &connection);

	void set_status(short code, std::string status);
	void prepare();
	virtual void reset();

	short code() const;

private:
	short m_code;
	std::string m_status;

	Connection &m_connection;
};
}

#endif // HTTP_REPLY_H
