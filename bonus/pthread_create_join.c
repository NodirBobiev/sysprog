#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#define ITERATIONS 100000


void* thread_worker (void *args)
{
    // dummy thread
    return NULL;
}

double benchmark()
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

    double result = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec))/(double)ITERATIONS;
    printf("ITERATIONS: %d \t %.2f ns per create/join pair\n", ITERATIONS, result);

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
        
    int M = 5;
    double arr[M];
    for (int j = 0; j < M; j ++ )
        arr[j] = benchmark();
    display(arr, 5);

}
