#ifndef HTTP_REPLY_H
#define HTTP_REPLY_H

#include "message.h"

namespace http {
class Connection;

class Reply : public virtual Message {

public:
	Reply(Connection &connection);

	void set_status(short code, std::string status);
	short code() const;
	bool send();

	// from message
	virtual void prepare();
	virtual void reset();

private:
	bool send_headers();
	bool send_body();

private:
	short m_code;
	std::string m_status;

	Connection &m_connection;
};
}

#endif // HTTP_REPLY_H
