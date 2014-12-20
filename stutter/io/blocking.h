#ifndef IO_BLOCKING_H
#define IO_BLOCKING_H

#include <stutter/io/strategy.h>

class BlockingIOStrategy : public virtual IOStrategy {

public:
    BlockingIOStrategy();
    virtual ~BlockingIOStrategy();

    // IO strategy
    virtual int safe_read (int fd, char *p, size_t sz);
    virtual int safe_write(int fd, const char *p, size_t sz);

    virtual int safe_sendfile(int out_fd, int in_fd, off_t *offset, size_t count); // sendfile

private:
    int m_fd;
};

#endif // IO_BLOCKING_H
