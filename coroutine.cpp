#include "coroutine.h"

#define COROUTINE_STACK_SIZE (16 * 1024)

using namespace std;

static void
entry_point(void *ptr)
{
	Coroutine *co = reinterpret_cast<Coroutine*>(ptr);
	co->run();
}

////////////////////////////////////////////////////////////

Coroutine::Coroutine(function<int (Coroutine&, void*)> fun, void *ptr)
		: m_fun(fun)
		, m_ptr(ptr)
		, m_state(INIT)
		, m_stack(0)
{
	// create new context based on current one:
	getcontext(&m_ctx); // copy current context into m_ctx
	create_stack();     // allocate stack for new context
	makecontext(&m_ctx, (void (*)())entry_point, 1, this); // create
}

Coroutine::~Coroutine()
{
	if (m_stack)
		delete[] m_stack;
}

Coroutine::State
Coroutine::state() const
{
	return m_state;
}

void
Coroutine::run()
{
	int v = m_fun(*this, m_ptr);
	m_state = DONE;
	yield(v);
}

int
Coroutine::resume()
{
	if (m_state == INIT)
		m_state = RUNNING;
	else if (m_state == DONE)
		return -1;

	ucontext_t prev_ctx = m_switcher.caller;
	swapcontext(&m_switcher.caller, &m_ctx); // switch to function
    // yield() brings us back here
	m_ctx = m_switcher.callee;               // save m_ctx
	m_switcher.caller = prev_ctx;            // save ctx for next use as "prev"

	return m_value;
}

void
Coroutine::yield(int value)
{
	// make available to .value()
	m_value = value;

	// switch out function context, get back to caller.
	swapcontext(&m_switcher.callee, &m_switcher.caller);
}

void
Coroutine::create_stack()
{
	m_stack = new char[COROUTINE_STACK_SIZE];

	m_ctx.uc_stack.ss_sp = (void*)m_stack;
	m_ctx.uc_stack.ss_size = COROUTINE_STACK_SIZE;
	m_ctx.uc_stack.ss_flags = 0;
	m_ctx.uc_link = 0;
}
