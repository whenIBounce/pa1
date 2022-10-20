#include "uthread.h"
#include "TCB.h"
#include <cassert>
#include <deque>
#include <set>
#include <map> 


using namespace std;

// Finished queue entry type
typedef struct finished_queue_entry {
  TCB *tcb;             // Pointer to TCB
  void *result;         // Pointer to thread result (output)
} finished_queue_entry_t;

// Join queue entry type
typedef struct join_queue_entry {
  TCB *tcb;             // Pointer to TCB
  int waiting_for_tid;  // TID this thread is waiting on
} join_queue_entry_t;

// You will need to maintain structures to track the state of threads
// - uthread library functions refer to threads by their TID so you will want
//   to be able to access a TCB given a thread ID
// - Threads move between different states in their lifetime (READY, BLOCK,
//   FINISH). You will want to maintain separate "queues" (doesn't have to
//   be that data structure) to move TCBs between different thread queues.
//   Starter code for a ready queue is provided to you
// - Separate join and finished "queues" can also help when supporting joining.
//   Example join and finished queue entry types are provided above

// Queues and data structures
static deque<TCB*> ready_queue;
static deque<TCB*> block_queue;

static deque<finished_queue_entry_t*> finished_queue;
static deque<join_queue_entry_t*> join_queue;

static int quantum;

static TCB *running;

static map<int,TCB*> tidMap;


// Interrupt Management --------------------------------------------------------

// Start a countdown timer to fire an interrupt
static void startInterruptTimer()
{
   struct itimerval new_timeset;

   // set reload  
   new_timeset.it_interval.tv_sec = 0; 
   // new ticker value 
   new_timeset.it_interval.tv_usec = 0;
   // store 
   new_timeset.it_value.tv_sec = 0; 
   // store
   new_timeset.it_value.tv_usec = running->getQuantum();
 
   setitimer(ITIMER_VIRTUAL, &new_timeset, NULL);
}

// Block signals from firing timer interrupt
static void disableInterrupts()
{ 
    // define a new mask set
    sigset_t mask_set;
    // first clear the set
    sigemptyset(&mask_set);
    // add the SIGVTALRM signal to our mask set
    sigaddset(&mask_set, SIGVTALRM);
    // block mask set
    sigprocmask(SIG_BLOCK, &mask_set, NULL);
}

// Unblock signals to re-enable timer interrupt
static void enableInterrupts()
{
      
    // define a new mask set  
    sigset_t mask_set;
    // first clear the set
    sigemptyset(&mask_set);
    //add the SIGVTALRM signal to our mask set
    sigaddset(&mask_set, SIGVTALRM);
    // unblock mask set
    sigprocmask(SIG_UNBLOCK, &mask_set, NULL);
}


// Ready Queue Management ------------------------------------------------------------

void addToReadyQueue(TCB *tcb)
{
        ready_queue.push_back(tcb);
}

// Removes and returns the first TCB on the ready queue
// NOTE: Assumes at least one thread on the ready queue
TCB* popFromReadyQueue()
{
        assert(!ready_queue.empty());

        TCB *ready_queue_head = ready_queue.front();
        ready_queue.pop_front();
        return ready_queue_head;
}

// Removes the thread specified by the TID provided from the ready queue
// Returns 0 on success, and -1 on failure (thread not in ready queue)
int removeFromReadyQueue(int tid)
{
        for (deque<TCB*>::iterator iter = ready_queue.begin(); iter != ready_queue.end(); ++iter)
        {
                if (tid == (*iter)->getId())
                {
                        ready_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

// Finished Queue Management ------------------------------------------------------------

int removeFromFinishedQueue(int tid)
{
        for (deque<finished_queue_entry*>::iterator iter = finished_queue.begin(); iter != finished_queue.end(); ++iter)
        {
                if (tid == (*iter)->tcb->getId())
                {
                        finished_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}

static finished_queue_entry* isTidInFinishedQueue(int tid) {
        for (deque<finished_queue_entry_t*>::iterator iter = finished_queue.begin(); iter != finished_queue.end(); ++iter)
        {
                if (tid == (*iter)->tcb->getId())
                {
                        return (*iter);
                }
        }

        // Thread not found
        return NULL;
}


// Join Queue Management ------------------------------------------------------------

int removeFromJoinQueue(join_queue_entry_t *entry)
{
        for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter)
        {
                if (entry->tcb->getId() == (*iter)->tcb->getId())
                {
                        join_queue.erase(iter);
                        return 0;
                }
        }

        // Thread not found
        return -1;
}


static join_queue_entry_t* getWaitingForTid(int tid) {
        for (deque<join_queue_entry_t*>::iterator iter = join_queue.begin(); iter != join_queue.end(); ++iter)
        {
                if (tid == (*iter)->waiting_for_tid)
                {
                        return (*iter);
                }
        }

        // Thread not found
        return NULL;
}


// Helper functions ------------------------------------------------------------

// check if running thread is not finished
static bool stillRunning(){
        return running->getState() == State::RUNNING;
}

//sa_handler in struct sigaction has to receive signum as a parameter
static void sigalHandler(int signum) {
    uthread_yield();
}
// Switch to the next ready thread
static void switchThreads()
{
    volatile int flag = 0;

    getcontext(&(running->_context));

    if (flag == 1) {
	startInterruptTimer();
	enableInterrupts();
	return;
    }

    disableInterrupts();

    flag = 1;
    // If running thread is not finished, add it to the end of READY queue 
    if (stillRunning()){
        running->setState(State::READY);
        addToReadyQueue(running);
    }
    
    // get next running thread
    TCB *nRunning = popFromReadyQueue();
    if(isTidInFinishedQueue(nRunning->getId()) != NULL) {
        removeFromReadyQueue(nRunning->getId()); 
        nRunning = popFromReadyQueue();
    }

    // set state and context for next running thread
    running = nRunning;
    running->setState(State::RUNNING);
    setcontext(&(nRunning->_context));

}

// Library functions -----------------------------------------------------------

// The function comments provide an (incomplete) summary of what each library
// function must do

// Starting point for thread. Calls top-level thread function
void stub(void *(*start_routine)(void *), void *arg)
{
    // start virtual timer
    startInterruptTimer(); 
    // Calls top-level thread function
    void *return_value = (*start_routine)(arg);
    // main thread exit
    uthread_exit(return_value);
}

int uthread_init(int quantum_usecs)
{
        // Initialize any data structures
        // Setup timer interrupt and handler
        // Create a thread for the caller (main) thread

    quantum = quantum_usecs;
    // Construct the new disposition
    struct sigaction newDisp;
    newDisp.sa_handler = sigalHandler;
    sigemptyset(&newDisp.sa_mask);
    newDisp.sa_flags = 0; 

    if(sigaction(SIGVTALRM, &newDisp, NULL) == -1){
        return -1;
    }

    TCB *init_thread = new TCB(0, State::RUNNING);
    init_thread->increaseQuantum(quantum);
    // tid for init thread would be 0
    tidMap.insert(pair<int, TCB*>(0, init_thread));
    getcontext(&init_thread->_context);
    //set running thread to be the init thread
    running = init_thread;

    startInterruptTimer();

    return 0;
}

int uthread_create(void* (*start_routine)(void*), void* arg)
{
        // Create a new thread and add it to the ready queue
	
        //check if num of threads exceeds the limit
        assert(tidMap.size()<MAX_THREAD_NUM);

        // enter critical section:
	disableInterrupts();

        // construct new thread
        int tid = tidMap.size() + 1;
        TCB *newThread = new TCB(tid, start_routine, arg, State::READY);
	newThread->increaseQuantum(quantum);
        tidMap.insert(pair<int, TCB*>(tid, newThread));
        // add new thread to READY queue
        addToReadyQueue(newThread);

        // exit critical section
	enableInterrupts();

	return tid;
}

int uthread_join(int tid, void **retval)
{
        // If the thread specified by tid is already terminated, just return
        // If the thread specified by tid is still running, block until it terminates
        // Set *retval to be the result of thread if retval != nullptr
    void *res;

    // If the thread specified by tid is still running, block until it terminates
    finished_queue_entry* finished_entry = isTidInFinishedQueue(tid);

    if (finished_entry == NULL) {

	disableInterrupts();

	running->setState(State::BLOCK);
	join_queue_entry_t join_entry = {running, tid};
	join_queue.push_back(&join_entry);

	enableInterrupts();

        uthread_yield();

        finished_entry = isTidInFinishedQueue(tid);
    }

    disableInterrupts();

    // Set *retval to be the result of thread if retval != nullptr
    if (retval != NULL) {
        *retval = finished_entry->result;
    }

    // deallocate tid
    removeFromFinishedQueue(tid);
    removeFromReadyQueue(tid);
    tidMap.erase(tid);

    enableInterrupts();
    return 1;
}

int uthread_yield(void)
{
    switchThreads();
    
    if (ready_queue.size() == 0){ 
        return 0;
    }else {
        return 1;
    }

}

void uthread_exit(void *retval)
{
        // If this is the main thread, exit the program
        // Move any threads joined on this thread back to the ready queue
        // Move this thread to the finished queue

    disableInterrupts();

    finished_queue_entry_t finished_entry = {running, retval};
    finished_queue.push_back(&finished_entry);

    running->setState(State::BLOCK);

    // Move any threads joined on this thread back to the ready queue
    // and remove from join queue
    join_queue_entry_t *joinedThreads = getWaitingForTid(running->getId());
    if (joinedThreads != NULL){
    	joinedThreads->tcb->setState(State::READY);
        addToReadyQueue(joinedThreads->tcb);
	removeFromJoinQueue(joinedThreads);
    }

    enableInterrupts();

    // schedule next READY thread to run
    uthread_yield(); 
}

int uthread_suspend(int tid)
{
        // Move the thread specified by tid from whatever state it is
        // in to the block queue

    if (tidMap.find(tid) != tidMap.end() && isTidInFinishedQueue(tid) == NULL) {

        disableInterrupts();
    
        TCB* tcb = tidMap[tid];
        tcb->setState(State::BLOCK); 

        removeFromReadyQueue(tid);

        if (running->getId() == tid){
                enableInterrupts();
                uthread_yield();
                return 1;
        }

        enableInterrupts();
        return 1;
    }
    return -1;
}

int uthread_resume(int tid)
{
        // Move the thread specified by tid back to the ready queue
        if (tidMap.find(tid) != tidMap.end()) {
                disableInterrupts();

                TCB* tcb = tidMap[tid];
                if (tcb->getState() != State::BLOCK) {
                        return -1;
                }
 
                tcb->setState(State::READY);
                addToReadyQueue(tcb);
                
                enableInterrupts();

                return 1;
    }
    return -1;
}

int uthread_self()
{
	return running->getId();
}

int uthread_get_total_quantums()
{
    return tidMap.size()*quantum;
    
}

int uthread_get_quantums(int tid)
{
    return tidMap[tid]->getQuantum();
}