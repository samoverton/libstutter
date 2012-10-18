#ifndef HTTP_REPLY_H
#define HTTP_REPLY_H

#include <stutter/http/message.h>

namespace http {
class Connection;

class Reply : public virtual Message {

public:
	Reply(Connection &cx);
	virtual ~Reply();

	void set_status(short code, std::string status);
	short code() const;
	bool send();
	bool set_file(const std::string filename);

	// from message
	virtual void prepare();
	virtual void reset();

protected:
	bool send_headers();

private:
	short m_code;
	std::string m_status;
};
}

#endif // HTTP_REPLY_H
