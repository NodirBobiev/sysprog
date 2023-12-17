#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>

#define ITERATIONS 10000000


volatile int shared_array[1000];


struct thread_worker_args{
    int thread_id;
    int distance;
};

void *thread_worker(void *args_ptr)
{
    struct thread_worker_args *args = (struct thread_worker_args*)(args_ptr);
    int distance = args->distance;  
    int thread_id = args->thread_id;
    volatile int i = 0;
    for (i = 0; i < ITERATIONS; i ++ ){
        shared_array[thread_id * distance] ++;
    }

    free(args_ptr);
}

long long benchmark(const char *label, int threads_count, int distance)
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
    return totalTime;
}

int compare(const void* a, const void* b) {
    return (*(long long*)a - *(long long*)b) > 0;
}
void display(long long arr[], int len) {
    qsort(arr, len, sizeof(long long), compare);
    printf("min: %lld \t median: %lld \t max: %lld\n\n", arr[0], arr[len / 2], arr[len-1]);
}

struct bench_param{
    const char* label;
    int threads_count;
    int distance;
};

struct bench_param parameters[]={
    {"100 mln increments with 1 thread", 1, 1},
    {"100 mln increments with 2 thread with close numbers", 2, 1},
    {"100 mln increments with 2 thread with distant numbers", 2, 20},
    {"100 mln increments with 3 thread with close numbers", 3, 1},
    {"100 mln increments with 3 thread with distant numbers", 3, 20},
};


int main(){
    
    
    int N = sizeof(parameters) / sizeof(parameters[0]);


    int M = 5;
    for (int i = 0; i < N; i ++ ){
        long long arr[M];
        for (int j = 0; j < M; j ++ )
            arr[j] = benchmark(parameters[i].label, parameters[i].threads_count, parameters[i].distance);
        display(arr, 5);
    }

    // benchmark("100 mln increments with 1 thread", 1, 1);
    // benchmark("100 mln increments with 2 thread with close numbers", 2, 1);
    // benchmark("100 mln increments with 2 thread with distant numbers", 2, 9);
    // benchmark("100 mln increments with 3 thread with close numbers", 3, 1);
    // benchmark("100 mln increments with 3 thread with distant numbers", 3, 9);
}