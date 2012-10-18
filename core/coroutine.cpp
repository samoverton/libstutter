#include <core/coroutine.h>

#ifdef USE_VALGRIND
#include <valgrind/valgrind.h>
#endif

#define COROUTINE_STACK_SIZE (32 * 1024)

using namespace std;

void
entry_point(void *ptr)
{
	Coroutine *co = reinterpret_cast<Coroutine*>(ptr);
	co->run();
}

////////////////////////////////////////////////////////////

Coroutine::Coroutine()
		: m_state(INIT)
		, m_stack(0)
{
	// create new context based on current one:
	create_stack();     // allocate stack for new context
	getcontext(&m_ctx); // copy current context into m_ctx
	makecontext(&m_ctx, (void (*)())entry_point, 1, this); // create
}

Coroutine::~Coroutine()
{
	if (m_stack) {
		delete[] m_stack;

#ifdef USE_VALGRIND
		VALGRIND_STACK_DEREGISTER(m_id);
#endif
	}
}

Coroutine::State
Coroutine::state() const
{
	return m_state;
}

void
Coroutine::run()
{
	int v = exec();
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

#ifdef USE_VALGRIND
	m_id = VALGRIND_STACK_REGISTER(m_stack, m_stack + COROUTINE_STACK_SIZE);
#endif
}
