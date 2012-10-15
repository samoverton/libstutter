#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <ostream>

class Log {
public:

	enum Level {
		DEBUG, INFO, NOTICE, WARN,
		ERROR, CRIT, ALERT, EMERG};

	Log(std::string filename, Level level);
	bool open();

private:
	std::string m_filename;
	std::ofstream m_out;
	Level m_level;

};

#endif // LOG_H
