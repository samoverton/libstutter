#include "log.h"
#include <iostream>

using namespace std;

Log::Log(string filename, Level level)
	: m_filename(filename)
	, m_level(level)
{
	m_null.open("/dev/null", ios::out);
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

ofstream &
Log::get(Level l)
{
	if (l >= m_level)
		return m_out;
	return m_null;
}
