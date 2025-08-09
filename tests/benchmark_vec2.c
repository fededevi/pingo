#include "benchmark_math.h"
#include "../math/vec2.h"

void benchmark_vec2_sum(int i) {
    Vec2i v1 = {i % 100, i % 200};
    Vec2i v2 = {i % 300, i % 400};
    Vec2i result = vector2ISum(v1, v2);
    (void)result; // Prevent optimization
}

void benchmark_vec2_itoF(int i) {
    Vec2i v = {i % 100, i % 200};
    Vec2f result = vecItoF(v);
    (void)result; // Prevent optimization
}

void benchmark_vec2_FtoI(int i) {
    Vec2f v = {(float)(i % 100), (float)(i % 200)};
    Vec2i result = vecFtoI(v);
    (void)result; // Prevent optimization
}

void benchmark_vec2() {
    printf("--- Vec2 Benchmarks ---\n");
    
    const int iterations = 1000000;
    
    benchmark_run("vector2ISum", iterations, benchmark_vec2_sum);
    benchmark_run("vecItoF", iterations, benchmark_vec2_itoF);
    benchmark_run("vecFtoI", iterations, benchmark_vec2_FtoI);
    
    printf("\n");
}
