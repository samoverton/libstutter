#include "coroutine.h"
#include <iostream>

using namespace std;

int f(Coroutine& co, void* ptr)
{
	co.yield(0);
	co.yield(1);
	return 42;
}

int
main(int argc, char *argv[])
{
	Coroutine co(f, (void*)0x1234);

	for (int i = 0; i < 4; ++i) {
		cout << "runnable = " << (co.state() != Coroutine::DONE) << endl;
		cout << "resume: " << co.resume() << endl << endl;
	}

	return 0;
}
