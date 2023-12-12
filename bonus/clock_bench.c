#include<stdio.h>
#include<time.h>
#include<bits/types/clockid_t.h>

#define ITERATIONS 50000000

void benchmark(const char* label, clockid_t clockType) {
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        struct timespec currentTime;
        clock_gettime(clockType, &currentTime);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long totalTime = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

    printf("%20s: \t %.2f ns per call\n", label, (double)totalTime / ITERATIONS);
}

int main() {
    benchmark("CLOCK_REALTIME", CLOCK_REALTIME);

    benchmark("CLOCK_MONOTONIC", CLOCK_MONOTONIC);

    benchmark("CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW);

    return 0;
}
