#include "uthread.h"
#include <iostream>

using namespace std;

typedef struct yeild_params {
    int start;
    int end;
    int quantum;
} yeild_prams;

void *worker1(void *arg) {
    int my_tid = uthread_self();
    yeild_params parameter = *(yeild_prams*)arg;
    printf("Thread %d STARTS\n", my_tid);

    for (int i = parameter.start; i <= parameter.end; i++) {
       printf("Thread %d starts at %d\n", my_tid,i);
        if ((i+1-parameter.start) % parameter.quantum == 0) {
            uthread_yield();
	        printf("NOW Thread %d is running\n", my_tid);
        }
    }
    printf("Finished running Thread  %d \n", my_tid);

    return NULL;
}

void test_yeild(){
    int quantum_usecs = 1000;   
    unsigned int thread_count = 2;

    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }
    yeild_params param1 = {1,7,2};
    threads[0] = uthread_create(worker1, &param1);
    yeild_params param2 = {5,8,3};
    threads[1] = uthread_create(worker1, &param2);

    cout<<"Threads Created\n";
    while(uthread_yield()){
        printf("Switching threads:\n");
    }
    printf("Exit\n");
    delete[] threads;

    
}
void test_join(){
    int quantum_usecs = 1000;   
    unsigned int thread_count = 2;

    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }

    return NULL;
}
// void test_suspend_and_resume();

int main(int argc, char *argv[]) {
    test_yeild();
    // test_join();
    // test_suspend_and_resume();
}