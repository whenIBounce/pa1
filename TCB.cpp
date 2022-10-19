#include "TCB.h"
#include <iostream>

TCB::TCB(int tid, void *(*start_routine)(void* arg), void *arg, State state)
{
	this->_tid = tid;
	this->_state = state;
    getcontext(&_context);
    
    _context.uc_stack.ss_sp = new char[STACK_SIZE];
    _context.uc_stack.ss_size = STACK_SIZE;
    _context.uc_stack.ss_flags = 0;

    // Set up the context to run f as the top-level thread function
    makecontext(&_context, (void(*)())stub, 2, start_routine, arg);
}

TCB::TCB(int tid, State state)
{
	this->_tid = tid;
	this->_state = state;
        //getcontext(&_context);
}

TCB::~TCB()
{
}

void TCB::setState(State state)
{
	this->_state = state;
}

State TCB::getState() const
{
	return _state;
}

int TCB::getId() const
{
	return this->_tid;
}

void TCB::increaseQuantum(int delta)
{
	_quantum += delta; // using this function to set the quanta of a thread.
}

int TCB::getQuantum() const
{
	return this->_quantum;
}
