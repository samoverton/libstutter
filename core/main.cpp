#include "server.h"

int
main(int argc, char *argv[])
{
	Server s("127.0.0.1", 8888);

	/*
	s.set_handler("/adhoc/api/",            jbirdForwardHandler);
	s.set_handler("/analytics/api/data/",   dataHandler);
	s.set_handler("/analytics/api/schema/", schemaHandler);
	*/

	s.start();

	return 0;
}