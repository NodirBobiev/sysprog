#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#define ITERATIONS 100000


void* thread_worker (void *args)
{
    // dummy thread
    return NULL;
}

void main()
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ITERATIONS; i ++){
        pthread_t thread = (pthread_t)malloc(sizeof(pthread_t));
        pthread_create(&thread, NULL, thread_worker, NULL);
        pthread_join(thread, NULL);
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    long long totalTime = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    printf("ITERATIONS: %d \t %.2f ns per create/join pair\n", ITERATIONS, (double)totalTime / ITERATIONS);
}
