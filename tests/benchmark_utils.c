#include "benchmark_math.h"

void benchmark_start(BenchmarkTimer* timer) {
    gettimeofday(&timer->start, NULL);
}

double benchmark_end(BenchmarkTimer* timer) {
    gettimeofday(&timer->end, NULL);
    double elapsed = (timer->end.tv_sec - timer->start.tv_sec) * 1000000.0;
    elapsed += (timer->end.tv_usec - timer->start.tv_usec);
    return elapsed / 1000000.0; // Return seconds
}

void benchmark_run(const char* name, int iterations, void (*func)(int)) {
    BenchmarkTimer timer;
    benchmark_start(&timer);
    for (int i = 0; i < iterations; i++) {
        func(i);
    }
    double elapsed = benchmark_end(&timer);
    printf("BENCHMARK %s: %d iterations in %.6f seconds (%.2f ops/sec)\n", 
           name, iterations, elapsed, iterations / elapsed);
}

void benchmark_run_timed(const char* name, double duration_seconds, void (*func)(int)) {
    BenchmarkTimer timer;
    benchmark_start(&timer);
    
    int iterations = 0;
    double elapsed;
    
    // Run for the specified duration
    do {
        func(iterations);
        iterations++;
        elapsed = benchmark_end(&timer);
    } while (elapsed < duration_seconds);
    
    printf("BENCHMARK %s: %d iterations in %.6f seconds (%.2f ops/sec)\n", 
           name, iterations, elapsed, iterations / elapsed);
}

void run_all_benchmarks() {
    printf("=== Pingo Math Library Benchmarks ===\n\n");
    
    benchmark_vec2();
    benchmark_vec3();
    benchmark_mat3();
    benchmark_mat4();
    benchmark_functions();
    
    printf("\n=== Benchmark Complete ===\n");
}
