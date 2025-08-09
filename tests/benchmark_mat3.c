#include "benchmark_math.h"
#include "../math/mat3.h"

void benchmark_mat3_identity(int i) {
    Mat3 result = mat3Identity();
    (void)result; // Prevent optimization
}

void benchmark_mat3_translate(int i) {
    Vec2f trans = {(float)i, (float)(i+1)};
    Mat3 result = mat3Translate(trans);
    (void)result; // Prevent optimization
}

void benchmark_mat3_rotate(int i) {
    Mat3 result = mat3Rotate((float)i * 0.01f);
    (void)result; // Prevent optimization
}

void benchmark_mat3_scale(int i) {
    Vec2f scale = {(float)i, (float)(i+1)};
    Mat3 result = mat3Scale(scale);
    (void)result; // Prevent optimization
}

void benchmark_mat3_multiply_vec(int i) {
    Vec2f v = {(float)i, (float)(i+1)};
    Mat3 m = mat3Identity();
    Vec2f result = mat3Multiply(&v, &m);
    (void)result; // Prevent optimization
}

void benchmark_mat3_multiply_mat(int i) {
    Mat3 m1 = mat3Identity();
    Mat3 m2 = mat3Translate((Vec2f){(float)i, (float)(i+1)});
    Mat3 result = mat3MultiplyM(&m1, &m2);
    (void)result; // Prevent optimization
}

void benchmark_mat3_determinant(int i) {
    Mat3 m = mat3Scale((Vec2f){(float)i, (float)(i+1)});
    F_TYPE result = mat3Determinant(&m);
    (void)result; // Prevent optimization
}

void benchmark_mat3_inverse(int i) {
    Mat3 m = mat3Scale((Vec2f){(float)i, (float)(i+1)});
    Mat3 result = mat3Inverse(&m);
    (void)result; // Prevent optimization
}

void benchmark_mat3() {
    printf("--- Mat3 Benchmarks ---\n");
    
    const int iterations = 1000000;
    
    benchmark_run("mat3Identity", iterations, benchmark_mat3_identity);
    benchmark_run("mat3Translate", iterations, benchmark_mat3_translate);
    benchmark_run("mat3Rotate", iterations, benchmark_mat3_rotate);
    benchmark_run("mat3Scale", iterations, benchmark_mat3_scale);
    benchmark_run("mat3Multiply (vec)", iterations, benchmark_mat3_multiply_vec);
    benchmark_run("mat3MultiplyM", iterations, benchmark_mat3_multiply_mat);
    benchmark_run("mat3Determinant", iterations, benchmark_mat3_determinant);
    benchmark_run("mat3Inverse", iterations, benchmark_mat3_inverse);
    
    printf("\n");
}
