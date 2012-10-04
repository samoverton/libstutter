#include "server.h"

int
main(int argc, char *argv[])
{
	Server s("127.0.0.1", 8888);
	s.start();

	return 0;
}
