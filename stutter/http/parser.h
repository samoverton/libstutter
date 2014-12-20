#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
#include <functional>

struct http_parser;
struct http_parser_settings;

namespace http {
class Request;
class Reply;
class Message;

class Parser {
public:
    typedef enum {REQUEST, RESPONSE} Mode;
    Parser(Mode m, http::Request *request, bool (*fun)(void*), void *ptr);
    Parser(Mode m, http::Reply *reply, bool (*fun)(void*), void *ptr);
    ~Parser();

    typedef enum {
        PARSE_OK,
        PARSE_FAILURE,
        PARSE_NEED_100_CONTINUE,
        PARSE_BODY_TOO_LARGE,
        PARSE_URI_TOO_LONG,
        PARSE_HANDLER_FAILURE} Error;

    Error add(const char *p, size_t sz);
    Error error() const;
    void reset();

    int http_major() const;
    int http_minor() const;

private:
    void configure_http_parser();
    int  save_last_header();
    void add_url_fragment (const char *p, size_t sz);
    void extract_query_string();
    void add_body_fragment(const char *at, size_t sz);
    void callback();

private:
    Mode m_mode;
    Error m_error;
    http::Request *m_request;
    http::Reply *m_reply;
    http::Message *m_msg;

    // callback (TODO: use std::function?)
    bool (*m_fun)(void*);
    void *m_fun_data;

    struct http_parser *m_parser;
    struct http_parser_settings *m_parserconf;
    std::string m_header_key;
    std::string m_header_val;
    bool m_header_gotval;

friend int _http_on_url_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_message_complete_cb(http_parser *p);
friend int _http_on_header_field_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_header_value_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_body_cb(http_parser *p, const char *at, size_t sz);
friend int _http_on_headers_complete_cb(http_parser *p);
};
}

#endif // HTTP_PARSER_H
