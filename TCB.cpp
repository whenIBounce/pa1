#include "TCB.h"
#include <iostream>

TCB::TCB(int tid, void *(*start_routine)(void* arg), void *arg, State state)
{
    _tid = tid;
	_state = state;

    getcontext(&_context);
    
    _context.uc_stack.ss_sp = new char[STACK_SIZE];
    _context.uc_stack.ss_size = STACK_SIZE;
    _context.uc_stack.ss_flags = 0;

    makecontext(&_context, (void(*)())stub, 2, start_routine, arg);
}


TCB::TCB(int tid, State state)
	:_tid(tid), _state(state) {}


TCB::~TCB()
{
}

void TCB::setState(State state)
{
    _state = state;
}

State TCB::getState() const
{
    return _state;
}

int TCB::getId() const
{
    return _tid;
}

void TCB::increaseQuantum(int quanta)
{
    _quantum += quanta;
}

int TCB::getQuantum( ) const
{
    return _quantum;
}

// int TCB::saveContext()
// {
// }

// void TCB::loadContext()
// {
// }
