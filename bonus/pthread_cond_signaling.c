#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#define ITERATIONS 1000000

pthread_mutex_t *mutex;
pthread_cond_t *cond;
int is_shutdown;

typedef int (*signal_f)(pthread_cond_t *);

struct thread_signaler_args
{
    signal_f signal;
};

void* thread_signaler(void *args_ptr)
{
    struct thread_signaler_args *args = (struct thread_signaler_args*)args_ptr;
    signal_f singal = args->signal;

    for (int i = 0; i < ITERATIONS; i ++ ){
        pthread_mutex_lock(mutex);
        singal(cond);
        pthread_mutex_unlock(mutex);
    }

    // tell waiters to shutdown by a broadcast.
    pthread_mutex_lock(mutex);
    is_shutdown = 1;
    pthread_cond_broadcast(cond);
    pthread_mutex_unlock(mutex);

    free(args_ptr);
    return NULL;
}

void* thread_waiter(void *args)
{
    pthread_mutex_lock(mutex);

    while(!is_shutdown)
        pthread_cond_wait(cond, mutex);

    pthread_mutex_unlock(mutex);
}


void benchmark(const char* label, signal_f signal, int waiters_count)
{
    mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(mutex, NULL);
    pthread_cond_init(cond, NULL);
    is_shutdown = 0;

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t)*(waiters_count + 1));

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(int i = 1; i <= waiters_count; i ++ ){
        pthread_create(&(threads[i]), NULL, thread_waiter, NULL);
    }

    struct thread_signaler_args *args = malloc(sizeof(struct thread_signaler_args));
    args->signal = signal;
    pthread_create(&(threads[0]), NULL, thread_signaler, args);

    for( int i = 0; i <= waiters_count; i ++ ){
        pthread_join(threads[i], NULL);
    }
    
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    long long totalTime = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

    printf("label: %s \t \t totalTime: %lld ns\n", label, totalTime);

    pthread_mutex_destroy(mutex);
    pthread_cond_destroy(cond);
    free(mutex);
    free(cond);
    free(threads);
}

int main(){
    benchmark("signal 1 wait-thread", pthread_cond_signal, 1);
    benchmark("broadcast 1 wait-thread", pthread_cond_broadcast, 1);

    benchmark("signal 2 wait-thread", pthread_cond_signal, 2);
    benchmark("broadcast 2 wait-thread", pthread_cond_broadcast, 2);

    benchmark("signal 3 wait-thread", pthread_cond_signal, 3);
    benchmark("broadcast 3 wait-thread", pthread_cond_broadcast, 3);

    benchmark("signal 4 wait-thread", pthread_cond_signal, 4);
    benchmark("broadcast 4 wait-thread", pthread_cond_broadcast, 4);
}