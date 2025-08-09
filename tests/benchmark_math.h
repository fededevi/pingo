#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

// Benchmark utilities
typedef struct {
    struct timeval start;
    struct timeval end;
} BenchmarkTimer;

// Function declarations
void benchmark_start(BenchmarkTimer* timer);
double benchmark_end(BenchmarkTimer* timer);
void benchmark_run(const char* name, int iterations, void (*func)(int));
void benchmark_run_timed(const char* name, double duration_seconds, void (*func)(int));

// Benchmark function prototypes
void benchmark_vec2();
void benchmark_vec3();
void benchmark_mat3();
void benchmark_mat4();
void benchmark_functions();

// Benchmark runner
void run_all_benchmarks();
