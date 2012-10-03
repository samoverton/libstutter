#include "coroutine.h"
#include "server.h"
#include <iostream>

using namespace std;

class NumberGenerator : public Coroutine
{
	virtual int exec()
	{
		yield(0);
		yield(1);
		return 42;
	}
};

int
main(int argc, char *argv[])
{
	NumberGenerator g;
	for (int i = 0; i < 4; ++i) {
		cout << "runnable = " << (g.state() != Coroutine::DONE) << endl;
		cout << "resume: " << g.resume() << endl << endl;
	}

	// Server s("127.0.0.1", 8888);
	// s.start();

	return 0;
}
