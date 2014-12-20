#ifndef IO_YIELDING_H
#define IO_YIELDING_H

#include <stutter/coroutine.h>
#include <stutter/io/strategy.h>

#include <event.h>
#include <string>
#include <map>

class YieldingIOStrategy : public Coroutine, public IOStrategy {

public:
    YieldingIOStrategy();
    virtual ~YieldingIOStrategy();

    int watched_fd() const;
    void watch_fd(int fd);
    struct event *event();

    enum Need {READ, WRITE, HALT};

    // IO strategy
    virtual int safe_read (int fd, char *p, size_t sz);
    virtual int safe_write(int fd, const char *p, size_t sz);

    virtual int safe_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);

private:
    int m_watched_fd;
    struct event m_ev;
};


#endif // IO_YIELDING_H
