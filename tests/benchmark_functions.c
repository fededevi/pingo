#include "benchmark_math.h"
#include "../math/fun.h"

void benchmark_edge_function(int i) {
    Vec2f a = {(float)i, (float)(i+1)};
    Vec2f b = {(float)(i+2), (float)(i+3)};
    Vec2f c = {(float)(i+4), (float)(i+5)};
    int result = edgeFunction(&a, &b, &c);
    (void)result; // Prevent optimization
}

void benchmark_is_clockwise(int i) {
    float result = isClockWise((float)i, (float)(i+1), (float)(i+2), 
                              (float)(i+3), (float)(i+4), (float)(i+5));
    (void)result; // Prevent optimization
}

void benchmark_orient2d(int i) {
    Vec2i a = {i, i+1};
    Vec2i b = {i+2, i+3};
    Vec2i c = {i+4, i+5};
    int result = orient2d(a, b, c);
    (void)result; // Prevent optimization
}

void benchmark_functions() {
    printf("--- Mathematical Functions Benchmarks ---\n");
    
    const int iterations = 1000000;
    
    benchmark_run("edgeFunction", iterations, benchmark_edge_function);
    benchmark_run("isClockWise", iterations, benchmark_is_clockwise);
    benchmark_run("orient2d", iterations, benchmark_orient2d);
    
    printf("\n");
}
