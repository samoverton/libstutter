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
	void close();
	std::ofstream &get(Level l);

private:
	std::string m_filename;
	Level m_level;

	// log back-ends
	std::ofstream m_out;
	std::ofstream m_null;
};

#endif // LOG_H
