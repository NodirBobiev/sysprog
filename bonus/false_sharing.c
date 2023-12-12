#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>

#define ITERATIONS 10000000


uint64_t shared_array[1000];


struct thread_worker_args{
    int thread_id;
    int distance;
};

void *thread_worker(void *args_ptr)
{
    struct thread_worker_args *args = (struct thread_worker_args*)(args_ptr);
    int distance = args->distance;  
    int thread_id = args->thread_id;

    for (int i = 0; i < ITERATIONS; i ++ ){
        volatile int dummy;
        if (dummy == 1){
            dummy = 0;
        }else{
            dummy = 1;
        }
        shared_array[thread_id * distance] ++;
    }

    free(args_ptr);
}

void benchmark(const char *label, int threads_count, int distance)
{
    pthread_t *threads = malloc(sizeof(pthread_t)*threads_count);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < threads_count; i ++ ){
        struct thread_worker_args *args = malloc(sizeof(struct thread_worker_args));
        args->distance = distance;
        args->thread_id = i;
        pthread_create(&(threads[i]), NULL, thread_worker, args);
    }

    for (int i = 0; i < threads_count; i ++ ){
        pthread_join(threads[i], NULL);
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    long long totalTime = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    printf("%-70s %lld ns \n", label, totalTime);

    free(threads);
}

int main(){
    benchmark("100 mln increments with 1 thread", 1, 1);
    benchmark("100 mln increments with 2 thread with close numbers", 2, 1);
    benchmark("100 mln increments with 2 thread with distant numbers", 2, 9);
    benchmark("100 mln increments with 3 thread with close numbers", 3, 1);
    benchmark("100 mln increments with 3 thread with distant numbers", 3, 9);
}