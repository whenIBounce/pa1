#include "TCB.h"

TCB::TCB(int tid, void *(*start_routine)(void* arg), void *arg, State state)
{
  _tid = tid;
  _state = state;
  _stack = (char*)malloc(STACK_SIZE);
  getcontext(&_context);
  _context.uc_stack.ss_sp = _stack;
  _context.uc_stack.ss_size = STACK_SIZE;
  _context.uc_stack.ss_flags = 0;
  makecontext(&context, (void(*)())stub,2,start_routine,arg);
}

TCB::~TCB()
{
  free(_stack);
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
  return _tid
}

void TCB::increaseQuantum()
{
  _quantum ++;

}

int TCB::getQuantum() const
{
  return _quantum;
}

int TCB::saveContext()
{
  return getcontext(&_context);
}

void TCB::loadContext()
{
  setcontext(&_context);
}
