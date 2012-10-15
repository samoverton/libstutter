#include "server.h"
#include "log.h"

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Server s("127.0.0.1", 8888);
	Log::setup("/tmp/stutter.log", Log::INFO);

	/*
	s.set_handler("/adhoc/api/",            jbirdForwardHandler);
	s.set_handler("/analytics/api/data/",   dataHandler);
	s.set_handler("/analytics/api/schema/", schemaHandler);
	*/

	s.start();

	return 0;
}
