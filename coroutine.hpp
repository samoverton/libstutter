#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <functional>
#include <ucontext.h>

#include <iostream>
using namespace std;

#define COROUTINE_STACK_SIZE (16 * 1024)

struct Switcher {
	ucontext_t caller;
	ucontext_t callee;
};

static void
entry_point(void *ptr);

class Coroutine {
public:
	Coroutine(Switcher &switcher,
			std::function<int (Coroutine&, void*)> fun, void *ptr)
		: m_switcher(switcher)
		, m_fun(fun)
		, m_ptr(ptr)
		, m_state(INIT)
	{
		getcontext(&m_ctx);
		create_stack();
		makecontext(&m_ctx, (void (*)())entry_point, 1, this);
	}

	~Coroutine() {
		delete[] m_stack;
	}

	typedef enum {
		INIT,
		RUNNING,
		DONE
	} State;

	int value() const;
	State state() const
	{
		return m_state;
	}

	void run()
	{
		int v = m_fun(*this, m_ptr);
		m_state = DONE;
		yield(v);
	}

	int resume()
	{
		if (m_state == INIT)
			m_state = RUNNING;
		else if (m_state == DONE)
			return -1;

		ucontext_t prev_ctx;
		
		prev_ctx = m_switcher.caller;
		swapcontext(&m_switcher.caller, &m_ctx);
		m_ctx = m_switcher.callee;
		m_switcher.caller = prev_ctx;

		return m_value;
	}

	void yield(int value)
	{
		m_value = value;
		swapcontext(&m_switcher.callee, &m_switcher.caller);
	}

private:

	void create_stack()
	{
		m_stack = new char[COROUTINE_STACK_SIZE];

		m_ctx.uc_stack.ss_sp = (void*)m_stack;
		m_ctx.uc_stack.ss_size = COROUTINE_STACK_SIZE;
		m_ctx.uc_stack.ss_flags = 0;
		m_ctx.uc_link = 0;
	}

private:
	std::function<int (Coroutine&, void*)> m_fun;
	void *m_ptr;

	int   m_value;
	State m_state;

	// ucontext
	Switcher &m_switcher;
	ucontext_t m_ctx;
	char *m_stack;
};

static void
entry_point(void *ptr)
{
	Coroutine *co = reinterpret_cast<Coroutine*>(ptr);
	co->run();
}


#endif // COROUTINE_HPP
