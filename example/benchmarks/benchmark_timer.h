#pragma once

#include <sys/time.h>

// Wall-clock timing for the example benchmarks. Kept separate from math's
// benchmark_math.h so that neither test directory depends on the other.
typedef struct {
  struct timeval start;
  struct timeval end;
} BenchmarkTimer;

void benchmark_start(BenchmarkTimer *timer);

// Seconds elapsed since benchmark_start.
double benchmark_end(BenchmarkTimer *timer);
