/*
How to run:
gcc atomic_increment.c -o main && ./main

*/

#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdatomic.h>
#include<time.h>

#define ITERATIONS 100000000

atomic_int counter;

void* thread_worker(void *order) {
    memory_order memoryOrder = *(memory_order *)order;
    while (atomic_fetch_add_explicit(&counter, 1, memoryOrder) < ITERATIONS);
    return NULL;
}

long long benchmark(const char *label, int threads_count, memory_order memoryOrder) {
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t)*threads_count);
    counter = 0;

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < threads_count; i++){
        pthread_create(&threads[i], NULL, thread_worker, &memoryOrder);
    }

    for (int i = 0; i < threads_count; i++){
        pthread_join(threads[i], NULL);
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    long long totalTime = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

    printf("%40s: \t %.2f ns per increment\n", label, (double)totalTime / ITERATIONS);

    free(threads);
    return totalTime;
}

int main() {
    benchmark("1 thread and relaxed order", 1, memory_order_relaxed);
    benchmark("2 threads and relaxed order", 2, memory_order_relaxed);
    benchmark("2 threads and sequentially consistent", 2, memory_order_seq_cst);
    benchmark("3 threads and relaxed order", 3, memory_order_relaxed);
    benchmark("3 threads and sequentially consistent", 3, memory_order_seq_cst);

    return 0;
}
