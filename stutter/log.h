#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <ostream>

class Log {
public:
    enum Level {
        TRACE, DEBUG, INFO, NOTICE, WARN,
        ERROR, CRIT, ALERT, EMERG};

    Log(std::string filename, Level level);
    static std::ostream &get(Level l);

    static bool setup(std::string filename, Level level);
    static bool enabled(Level level);

private:
    bool open();
    void close();

    Level level() const;
    std::ostream &stream();

private:
    std::string m_filename;
    Level m_level;

    // log back-end
    std::ofstream m_out;
};

#endif // LOG_H
