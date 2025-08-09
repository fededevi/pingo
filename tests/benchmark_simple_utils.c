#include "benchmark_math.h"
#include <sys/time.h>
#include <stdio.h>

void benchmark_start(BenchmarkTimer* timer) {
    gettimeofday(&timer->start, NULL);
}

double benchmark_end(BenchmarkTimer* timer) {
    gettimeofday(&timer->end, NULL);
    double start_time = timer->start.tv_sec + timer->start.tv_usec / 1000000.0;
    double end_time = timer->end.tv_sec + timer->end.tv_usec / 1000000.0;
    return end_time - start_time;
}
