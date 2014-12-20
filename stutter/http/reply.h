#ifndef HTTP_REPLY_H
#define HTTP_REPLY_H

#include <stutter/http/message.h>

namespace http {

class Reply : public virtual Message {

public:
    Reply();
    virtual ~Reply();

    void set_status(short code, std::string status);
    void set_http_version(int major, int minor);
    short code() const;
    const std::string& status() const;
    bool send();
    bool set_file(const std::string filename);

    // from message
    virtual void prepare();
    virtual void reset();

protected:
    bool send_headers();

private:
    short m_code;
    int m_http_major;
    int m_http_minor;
    std::string m_status;
};
}

#endif // HTTP_REPLY_H
