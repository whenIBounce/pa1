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
    int quantum = *(int*)arg;
    printf("Thread %d STARTS\n", my_tid);

    for (int i = 0; i <= 10; i++) {
       printf("Thread %d starts at %d\n", my_tid,i);
        if ((i+1) % quantum == 0) {
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
    int *param1 = (int *)2;
    int *param2 = (int *)3;

    threads[0] = uthread_create(worker1, &param1);
    threads[1] = uthread_create(worker1, &param2);

    cout<<"Threads Created\n";
    while(uthread_yield()){
        printf("Switching threads:\n");
    }
    printf("Exit\n");
    delete[] threads;

    
}
// void test_join(){
//     int quantum_usecs = 1000;   
//     unsigned int thread_count = 2;

//     int *threads = new int[thread_count];

//     // Init user thread library
//     int ret = uthread_init(quantum_usecs);
//     if (ret != 0) {
//         cerr << "uthread_init FAIL!\n" << endl;
//         exit(1);
//     }

//     return NULL;
// }
// void test_suspend_and_resume();

int main(int argc, char *argv[]) {
    test_yeild();
    // test_join();
    // test_suspend_and_resume();
}