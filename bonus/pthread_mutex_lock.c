#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#define ITERATIONS 10000000

pthread_mutex_t *mutex;
int counter;


void* thread_worker (void *args)
{
    while (counter < ITERATIONS) {
        pthread_mutex_lock(mutex);
        counter ++;
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}

double benchmark(int threads_count)
{
    pthread_t *threads = malloc(sizeof(pthread_t)*threads_count);
    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    counter = 0;

    pthread_mutex_init(mutex, NULL);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < threads_count; i ++ ){
        pthread_create(&(threads[i]), NULL, thread_worker, NULL);
    }

    for (int i = 0; i < threads_count; i ++ ){
        pthread_join(threads[i], NULL);
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    double result = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec))/(double)ITERATIONS;
    printf("threads_count: %d \t ITERATIONS: %d \t %.2f ns per lock/unlock pair\n", threads_count, ITERATIONS, result);

    pthread_mutex_destroy(mutex);
    free(threads);
    free(mutex);
    return result;
}

int compare(const void* a, const void* b) {
    return (*(double*)a - *(double*)b) > 0;
}
void display(double arr[], int len) {
    qsort(arr, len, sizeof(double), compare);
    printf("min: %.2f \t median: %.2f \t max: %.2f\n\n", arr[0], arr[len / 2], arr[len-1]);
}

int main(){
    for (int i = 1; i <= 10; i ++ ){
        int M = 5;
        double arr[M];
        for (int j = 0; j < M; j ++ )
            arr[j] = benchmark(i);
        display(arr, 5);
    }

}