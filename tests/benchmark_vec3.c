#include "benchmark_math.h"
#include "../math/vec3.h"

void benchmark_vec3_constructor(int i) {
    Vec3f result = vec3f((float)i, (float)(i+1), (float)(i+2));
    (void)result; // Prevent optimization
}

void benchmark_vec3_mul(int i) {
    Vec3f v = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f result = vec3fmul(v, 2.0f);
    (void)result; // Prevent optimization
}

void benchmark_vec3_sumV(int i) {
    Vec3f v1 = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f v2 = {(float)(i+3), (float)(i+4), (float)(i+5)};
    Vec3f result = vec3fsumV(v1, v2);
    (void)result; // Prevent optimization
}

void benchmark_vec3_subV(int i) {
    Vec3f v1 = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f v2 = {(float)(i+3), (float)(i+4), (float)(i+5)};
    Vec3f result = vec3fsubV(v1, v2);
    (void)result; // Prevent optimization
}

void benchmark_vec3_sum(int i) {
    Vec3f v = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f result = vec3fsum(v, 5.0f);
    (void)result; // Prevent optimization
}

void benchmark_vec3_dot(int i) {
    Vec3f a = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f b = {(float)(i+3), (float)(i+4), (float)(i+5)};
    F_TYPE result = vec3Dot(a, b);
    (void)result; // Prevent optimization
}

void benchmark_vec3_cross(int i) {
    Vec3f a = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f b = {(float)(i+3), (float)(i+4), (float)(i+5)};
    Vec3f result = vec3Cross(a, b);
    (void)result; // Prevent optimization
}

void benchmark_vec3_normalize(int i) {
    Vec3f v = {(float)i, (float)(i+1), (float)(i+2)};
    Vec3f result = vec3Normalize(v);
    (void)result; // Prevent optimization
}

void benchmark_vec3() {
    printf("--- Vec3 Benchmarks ---\n");
    
    const int iterations = 1000000;
    
    benchmark_run("vec3f constructor", iterations, benchmark_vec3_constructor);
    benchmark_run("vec3fmul", iterations, benchmark_vec3_mul);
    benchmark_run("vec3fsumV", iterations, benchmark_vec3_sumV);
    benchmark_run("vec3fsubV", iterations, benchmark_vec3_subV);
    benchmark_run("vec3fsum", iterations, benchmark_vec3_sum);
    benchmark_run("vec3Dot", iterations, benchmark_vec3_dot);
    benchmark_run("vec3Cross", iterations, benchmark_vec3_cross);
    benchmark_run("vec3Normalize", iterations, benchmark_vec3_normalize);
    
    printf("\n");
}
