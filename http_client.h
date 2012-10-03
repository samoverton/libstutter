#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

class HttpClient {
public:
	HttpClient(int fd);

private:
	int m_fd;
};

#endif // HTTP_CLIENT_H
