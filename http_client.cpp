#include "http_client.h"
#include "unistd.h"

#include <iostream>
using namespace std;

HttpClient::HttpClient(Server &server, int fd)
	: m_server(server)
	, m_fd(fd)
{
}

int
HttpClient::safe_read(char *p, size_t sz)
{
	yield((int)Need::READ);
	return read(m_fd, p, sz);
}

int
HttpClient::safe_write(char *p, size_t sz)
{
	yield((int)Need::WRITE);
	return write(m_fd, p, sz);
}

int
HttpClient::exec()
{
	while(true)
	{
		char buffer[5];
		int recvd = safe_read(buffer, sizeof(buffer));
		if (recvd <= 0)
			return (int)Need::HALT;
		
		int done = 0;
		while (done < recvd)
		{
			int sent = safe_write(buffer + done, recvd - done);
			if (sent <= 0)
				return (int)Need::HALT;
			done += sent;
		}
	}

	return (int)Need::HALT;
}

int
HttpClient::fd() const
{
	return m_fd;
}

struct event *
HttpClient::event()
{
	return &m_ev;
}

Server &
HttpClient::server()
{
	return m_server;
}
