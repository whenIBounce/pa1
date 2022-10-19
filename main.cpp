// #include "uthread.h"
// #include <iostream>

// using namespace std;

// void *worker(void *arg) {
//     int my_tid = uthread_self();
//     int points_per_thread = *(int*)arg;

//     unsigned long local_cnt = 0;
//     unsigned int rand_state = rand();
//     for (int i = 0; i < points_per_thread; i++) {
//         double x = rand_r(&rand_state) / ((double)RAND_MAX + 1) * 2.0 - 1.0;
//         double y = rand_r(&rand_state) / ((double)RAND_MAX + 1) * 2.0 - 1.0;
//         if (x * x + y * y < 1)
//             local_cnt++;
//     }

//     // NOTE: Parent thread must deallocate
//     unsigned long *return_buffer = new unsigned long;
//     *return_buffer = local_cnt;
//     return return_buffer;
// }

// int main(int argc, char *argv[]) {
//     // Default to 1 ms time quantum
//     int quantum_usecs = 1000;

//     if (argc < 3) {
//         cerr << "Usage: ./pi <total points> <threads> [quantum_usecs]" << endl;
//         cerr << "Example: ./pi 100000000 8" << endl;
//         exit(1);
//     } else if (argc == 4) {
//         quantum_usecs = atoi(argv[3]);
//     }
//     unsigned long totalpoints = atol(argv[1]);
//     int thread_count = atoi(argv[2]);

//     int *threads = new int[thread_count];
//     int points_per_thread = totalpoints / thread_count;

//     // Init user thread library
//     int ret = uthread_init(quantum_usecs);
//     if (ret != 0) {
//         cerr << "uthread_init FAIL!\n" << endl;
//         exit(1);
//     }

//     srand(time(NULL));

//     // Create threads
//     for (int i = 0; i < thread_count; i++) {
//         int tid = uthread_create(worker, &points_per_thread);
//         threads[i] = tid;
//     }

//     // Wait for all threads to complete
//     unsigned long g_cnt = 0;
//     for (int i = 0; i < thread_count; i++) {
//         // Add thread result to global total
//         unsigned long *local_cnt;
//         uthread_join(threads[i], (void**)&local_cnt);
//         g_cnt += *local_cnt;

//         // Deallocate thread result
//         delete local_cnt;
//     }

//     delete[] threads;

//     cout << "Pi: " << (4. * (double)g_cnt) / ((double)points_per_thread * thread_count) << endl;

//     return 0;
// }
// #include "uthread.h"
// #include <iostream>

// using namespace std;

// void *worker(void *arg) {
//     int my_tid = uthread_self();
//     int parameter = *(int*)arg;

//     printf("thread id: %d, parameter: %d\n", my_tid, parameter);
//     unsigned long return_zero = 0;
//     unsigned long *return_buffer = new unsigned long;

//     *return_buffer = my_tid;

//     return return_buffer;
// }

// int main(int argc, char *argv[]) {
//     int quantum_usecs = 1000;
    
//     unsigned int thread_count = 5;
//     int *threads = new int[thread_count];

//     // Init user thread library
//     int ret = uthread_init(quantum_usecs);
//     if (ret != 0) {
//         cerr << "uthread_init FAIL!\n" << endl;
//         exit(1);
//     }

//     srand(time(NULL));
//     //unsigned int random_parameter = 0;
    
//     int *random_numbers = (int *)malloc(thread_count * sizeof(int));

//     // Create threads
//     for (int i = 0; i < thread_count; i++) {
//         random_numbers[i] = rand();
//         printf("random number generated: %d\n", random_numbers[i]);
//         int tid = uthread_create(worker, &random_numbers[i]);
//         threads[i] = tid;
//     }

//     // Wait for all threads to complete
//     for (int i = 0; i < thread_count; i++) {
//         // Add thread result to global total
//         unsigned long *return_value;
//         uthread_join(threads[i], (void**)&return_value);
//         //
//         // Deallocate thread result
//         delete return_value;
//     }

//     delete[] threads;

//     return 0;
// }
#include "uthread.h"
#include <iostream>

using namespace std;

void *worker(void *arg) {
    int my_tid = uthread_self();
    int parameter = *(int*)arg;

    printf("thread id: %d, parameter: %d\n", my_tid, parameter);
    unsigned long return_zero = 0;
    int *return_buffer = new int;
    
    
    if (parameter > my_tid){
      *return_buffer = parameter - my_tid;
    }else{
      *return_buffer = my_tid - parameter;
    }

    for(int i = 0; i < 10000000 + *return_buffer; i++);

    return return_buffer;
}

int main(int argc, char *argv[]) {
    int quantum_usecs = 1000;
    
    unsigned int thread_count = 5;
    int *threads = new int[thread_count];

    // Init user thread library
    int ret = uthread_init(quantum_usecs);
    if (ret != 0) {
        cerr << "uthread_init FAIL!\n" << endl;
        exit(1);
    }

    srand(time(NULL));
    //unsigned int random_parameter = 0;
    
    int *random_numbers = (int *)malloc(thread_count * sizeof(int));

    // Create threads
    for (int i = 0; i < thread_count; i++) {
        random_numbers[i] = rand();
        printf("random number generated: %d\n", random_numbers[i]);
        int tid = uthread_create(worker, &random_numbers[i]);
        threads[i] = tid;
    }

    // Wait for all threads to complete
    for (int i = 0; i < thread_count; i++) {
        // Add thread result to global total
        int *return_value;
        uthread_join(threads[i], (void**)&return_value);
        //
        // Deallocate thread result
        printf("return value: %d\n", *return_value);
        delete return_value;
    }

    delete[] threads;

    return 0;
}
