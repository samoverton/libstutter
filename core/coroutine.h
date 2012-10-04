#ifndef COROUTINE_H
#define COROUTINE_H

#include <functional>
#include <ucontext.h>

class Coroutine {
public:
	Coroutine();
	~Coroutine();

	typedef enum {INIT, RUNNING, DONE} State;
	State state() const;
	int resume();
	void yield(int value);

protected:
	virtual int exec() = 0;
	void run();

private:
	void create_stack();

	struct Switcher {
		ucontext_t caller;
		ucontext_t callee;
	};

private:
	int   m_value;
	State m_state;

	// ucontext
	Switcher m_switcher;
	ucontext_t m_ctx;
	char *m_stack;

friend void entry_point(void*);
};


#endif // COROUTINE_H
