#include "uthread.h"
#include <iostream>

using namespace std;

typedef struct self_params{
    int tid;
}self_params;

typedef struct join_params {
    int start;
    int end;
    int quantum;
} join_params;

typedef struct suspend_params {
    int yeild_mod;
    int suspended_thread;
} suspend_params;


void *worker1(void *arg) {
    int my_tid = uthread_self();
    int quantum = *(int*)arg;
    printf("Thread %d STARTS\n", my_tid);

    for (int i = 0; i < 10; i++) {
       printf("Thread %d starts at %d\n", my_tid,i);
        if ((i+1) % quantum == 0) {
            uthread_yield();
	        printf("NOW Thread %d is running\n", my_tid);
        }
    }
    printf("Finished running Thread  %d \n", my_tid);

    return NULL;
}

void *worker2(void *arg){
    int my_tid = uthread_self();
    self_params main_thread = *(self_params*)arg;
    if(my_tid == main_thread.tid){
        printf("Test uthread_self() for thread %d succeeds\n", my_tid);
    }
    else{
         printf("Test uthread_self() for thread %d fails\n", my_tid);
    }
    return NULL;
}


void *worker3(void *arg) {
     int my_tid = uthread_self();
    join_params param = *(join_params*)arg;
    printf("Thread %d STARTS\n", my_tid);

    for (int i = param.start; i < param.end; i++) {
       printf("Thread %d starts at %d\n", my_tid,i);
        if ((i+1-param.start) % param.quantum == 0) {
            uthread_yield();
        }
    }
    return NULL;
}

void *worker4(void *arg) {
    int my_tid = uthread_self();
    suspend_params param = *(suspend_params*)arg;

    if(param.suspended_thread != -1){
        printf("Thread %d is suspended by thread %d\n", param.suspended_thread, my_tid);
        uthread_suspend(param.suspended_thread);
    }
    printf("Thread %d STARTS\n", my_tid);

    for (int i = 0; i < 10; i++) {
       printf("Thread %d starts at %d\n", my_tid,i);
       for (long j = 0; j < 10000000; j++);
    }

    return NULL;
}

void test_yeild(){
    int quantum_usecs = 1000;   
    unsigned int thread_count = 2;

    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        printf("uthread_init FAIL!\n");
        exit(1);
    }

    int *param1 = (int *)2;
    int *param2 = (int *)3;

    threads[0] = uthread_create(worker1, &param1);
    threads[1] = uthread_create(worker1, &param2);

     for (int i = 0; i < 2; i ++) {
        uthread_join(threads[i], NULL);
    }

    printf("EXIT\n");
    printf("\n");

    delete[] threads;
}

void test_self(){

 int quantum_usecs = 1000;   
    unsigned int thread_count = 2;

    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        printf("uthread_init FAIL!\n");
        exit(1);
    }
    self_params thread_id = {0};
    threads[0] = uthread_create(worker2, &thread_id);
    thread_id.tid = threads[0];

    self_params thread_id_2 = {0};
    threads[1] = uthread_create(worker2, &thread_id_2);
    // thread_id_2.tid = threads[1];

    for (int i = 0; i < 2; i++) {
	    uthread_join(threads[i], NULL);
    }

    printf("EXIT\n");
    printf("\n");
    delete[] threads;
}



void test_join(){
    int quantum_usecs = 10000;   
    unsigned int thread_count = 3;

    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        printf("uthread_init FAIL!\n");
        exit(1);
    }
    join_params param1 = {1,2,2};
    join_params param2 = {13,26,4};
    join_params param3 = {7,19,3};

    threads[0] = uthread_create(worker3, &param1);
    threads[1] = uthread_create(worker3, &param2);
    threads[2] = uthread_create(worker3, &param3);

    for (int i = 0; i < 3; i++) {
	    printf("Thread %d is joininig\n", threads[i]);
	    uthread_join(threads[i], NULL);
    }

    printf("EXIT\n");
    printf("\n");
    delete[] threads;   
}

void test_suspend_and_resume(){

    int quantum_usecs = 1000;   
    unsigned int thread_count = 3;

    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        printf("uthread_init FAIL!\n");
        exit(1);
    }

    suspend_params param1 = {3, -1};
    suspend_params param2 = {1, -1};
    suspend_params param3 = {2, -1};

    threads[0] = uthread_create(worker4, &param1);
    threads[1] = uthread_create(worker4, &param2);
    threads[2] = uthread_create(worker4, &param3);
    param1.suspended_thread = threads[1];



     for (int i = 0; i < 3; i ++) {
        uthread_join(threads[i], NULL);
        if(i == 0){
            printf("Resume thread %d\n", threads[1]);
            uthread_resume(threads[1]);
        }
    }

    printf("EXIT\n");
    printf("\n");

    delete[] threads;

}

// void test_suspend_by_different_thread_and_resume() {
//     // Default to 1 ms time quantum
//     int quantum_usecs = 100; // keeping quantum_usecs large enough to let the code only work based on yield

//     int *threads = new int[3];
//     // Init user thread library
//     int ret = uthread_init(quantum_usecs);
//     unsigned long *result;
//     if (ret != 0) {
//         cerr << "uthread_init FAIL!\n" << endl;
//         exit(1);
//     }
    
//     arguments_for_worker args = {1,3,2,-1};
//     threads[0] = uthread_create(worker5, &args);
//     arguments_for_worker args1 = {11,15,3,-1};
//     threads[1] = uthread_create(worker5, &args1);
//     arguments_for_worker args2 = {21,25,4,-1};
//     threads[2] = uthread_create(worker5, &args2);
//     args.thread_to_be_suspended = threads[1];

//     cout<<"Threads Created\n";
//     for (int i = 0; i < 3; i++) {
// 	    cout << "Joining on - " << threads[i] << "\n";
// 	    uthread_join(threads[i], NULL);
// 	    if (i == 0) {
// 		    cout<<"Resuming thread " << threads[1] << "\n";
// 		    uthread_resume(threads[1]);
//             }
//     }

//     cout<<"Exiting"<<endl;
//     delete[] threads;
// }

int main(int argc, char *argv[]) {
    printf("TEST YEILD FUNCTION: \n");
    test_yeild();
    printf("TEST SELF FUNCTION: \n");
    test_self();
    printf("TEST JOIN FUNCTION: \n");
    test_join();
    printf("TEST SUSPEND AND RESUME FUNCTION: \n");
    test_suspend_and_resume();
    // test_suspend_by_different_thread_and_resume();
}