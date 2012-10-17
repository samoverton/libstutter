#include "log.h"
#include <iostream>

using namespace std;

static Log m_log ("/dev/stderr", Log::INFO);
static Log m_null("/dev/null", Log::INFO);

Log::Log(string filename, Level level)
	: m_filename(filename)
	, m_level(level)
{
}

bool
Log::open()
{
	m_out.open(m_filename.c_str(), ios::out | ios::app);
	if (!m_out.is_open()) {
		cerr << "Couldn't open log file " << m_filename << " for writing" << endl;
		return false;
	}
	return true;
}

void
Log::close()
{
	m_out.close();
}

ostream &
Log::get(Level l)
{
	if (l >= m_log.level()) {
		return m_log.stream();
	}
	return m_null.stream();
}

ostream &
Log::stream()
{
	return m_out;
}

Log::Level
Log::level() const
{
	return m_level;
}

bool
Log::setup(std::string filename, Level level)
{
	m_log.close();
	m_log.m_filename = filename;
	m_log.m_level = level;
	return m_log.open();
}
