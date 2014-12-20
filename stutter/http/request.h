#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stutter/http/message.h>

class IOStrategy;

namespace http {
class Reply;
class Parser;

class Request : public virtual Message {
public:
    Request();
    Request(const Request &request);
    virtual ~Request();
    const std::string &url() const;
    void add_url_fragment(const char *at, size_t sz);
    void extract_query_string();
    void set_url(std::string s);
    void set_host(std::string host);

    typedef enum {GET, POST, PUT, HEAD, DELETE} Verb;
    void set_verb(Verb v);
    Verb verb() const;
    std::string verb_str() const;

    typedef std::map<std::string, std::string> QueryString;
    const QueryString &query_string() const;

    bool send_continue();
    bool require_100_continue() const;

    // from message
    virtual void prepare();
    virtual void reset();

private:
    bool send_data(Parser &parser, Reply &reply);
    void release_socket();

protected:
    bool send_body(Parser &parser, Reply &reply);

private:
    int m_fd;
    Verb m_verb;
    std::string m_url;
    std::string m_host;

    QueryString m_querystring;

    // upload
    bool m_require_100;
};

}
#endif // HTTP_REQUEST_H
