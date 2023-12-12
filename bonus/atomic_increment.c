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

double benchmark(const char *label, int threads_count, memory_order memoryOrder) {
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

    double result = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec))/(double)ITERATIONS;

    free(threads);

    printf("%25s: \t %.2f ns per iteration\n", label, result);
    return result;
}

int compare(const void* a, const void* b) {
    return (*(double*)a - *(double*)b) > 0;
}
void display(double arr[], int len) {
    qsort(arr, len, sizeof(double), compare);
    printf("min: %.2f \t median: %.2f \t max: %.2f\n\n", arr[0], arr[len / 2], arr[len-1]);
}

struct bench_param{
    const char* label;
    int threads_count;
    memory_order order;
};

struct bench_param parameters[]={
    {"1 thread and relaxed order", 1, memory_order_relaxed},
    {"2 threads and relaxed order", 2, memory_order_relaxed},
    {"2 threads and sequentially consistent", 2, memory_order_seq_cst},
    {"3 threads and relaxed order", 3, memory_order_relaxed},
    {"3 threads and sequentially consistent", 3, memory_order_seq_cst}
};



int main() {

    int N = sizeof(parameters) / sizeof(parameters[0]);

    for (int i = 0; i < N; i ++ ){
        double arr[5];
        for (int j = 0; j < 5; j ++ )
            arr[j] = benchmark(parameters[i].label, parameters[i].threads_count, parameters[i].order);
        display(arr, 5);
    }

    // benchmark("1 thread and relaxed order", 1, memory_order_relaxed);
    // benchmark("2 threads and relaxed order", 2, memory_order_relaxed);
    // benchmark("2 threads and sequentially consistent", 2, memory_order_seq_cst);
    // benchmark("3 threads and relaxed order", 3, memory_order_relaxed);
    // benchmark("3 threads and sequentially consistent", 3, memory_order_seq_cst);

    return 0;
}
