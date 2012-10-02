#ifndef COROUTINE_H
#define COROUTINE_H

#include <functional>
#include <ucontext.h>

class Coroutine {
public:
	Coroutine(std::function<int (Coroutine&, void*)> fun, void *ptr);
	~Coroutine();

	typedef enum {
		INIT,
		RUNNING,
		DONE
	} State;

	State state() const;

	void run();
	int resume();
	void yield(int value);

private:
	void create_stack();

	struct Switcher {
		ucontext_t caller;
		ucontext_t callee;
	};

private:
	std::function<int (Coroutine&, void*)> m_fun;
	void *m_ptr;

	int   m_value;
	State m_state;

	// ucontext
	Switcher m_switcher;
	ucontext_t m_ctx;
	char *m_stack;
};



#endif // COROUTINE_H
