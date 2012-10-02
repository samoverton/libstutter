#include "coroutine.hpp"

using namespace std;

int f(Coroutine& co, void* ptr) {
	
	co.yield(0);
	co.yield(1);
	return 42;
}

int
main(int argc, char *argv[])
{
	Switcher switcher;
	Coroutine co(switcher, f, (void*)0x1234);

	cout << "resume: " << co.resume() << endl;
	cout << "state = " << co.state() << endl;

	cout << "resume: " << co.resume() << endl;
	cout << "state = " << co.state() << endl;

	cout << "resume: " << co.resume() << endl;
	cout << "state = " << co.state() << endl;
	
	cout << "resume: " << co.resume() << endl;
	cout << "state = " << co.state() << endl;

	return 0;
}
