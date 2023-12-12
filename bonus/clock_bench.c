#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<bits/types/clockid_t.h>

#define ITERATIONS 50000000

double benchmark(const char* label, clockid_t clockType) {
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        struct timespec currentTime;
        clock_gettime(clockType, &currentTime);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double result = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec))/(double)ITERATIONS;

    printf("%20s: \t %.2f ns per call\n", label, result);
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
    clockid_t clockType;
};

struct bench_param parameters[]={
    {"CLOCK_REALTIME", CLOCK_REALTIME},
    {"CLOCK_MONOTONIC", CLOCK_MONOTONIC},
    {"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW},
};

int main() {
    int N = sizeof(parameters) / sizeof(parameters[0]);


    int M = 5;
    for (int i = 0; i < N; i ++ ){
        double arr[M];
        for (int j = 0; j < M; j ++ )
            arr[j] = benchmark(parameters[i].label, parameters[i].clockType);
        display(arr, 5);
    }

    return 0;
}
