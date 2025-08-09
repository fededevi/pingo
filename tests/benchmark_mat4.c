#include "benchmark_math.h"
#include "../math/mat4.h"

void benchmark_mat4_identity(int i) {
    Mat4 result = mat4Identity();
    (void)result; // Prevent optimization
}

void benchmark_mat4_translate(int i) {
    Vec3f trans = {(float)i, (float)(i+1), (float)(i+2)};
    Mat4 result = mat4Translate(trans);
    (void)result; // Prevent optimization
}

void benchmark_mat4_rotateX(int i) {
    Mat4 result = mat4RotateX((float)i * 0.01f);
    (void)result; // Prevent optimization
}

void benchmark_mat4_rotateY(int i) {
    Mat4 result = mat4RotateY((float)i * 0.01f);
    (void)result; // Prevent optimization
}

void benchmark_mat4_rotateZ(int i) {
    Mat4 result = mat4RotateZ((float)i * 0.01f);
    (void)result; // Prevent optimization
}

void benchmark_mat4_scale(int i) {
    Vec3f scale = {(float)i, (float)(i+1), (float)(i+2)};
    Mat4 result = mat4Scale(scale);
    (void)result; // Prevent optimization
}

void benchmark_mat4_multiply_vec2(int i) {
    Vec2f v = {(float)i, (float)(i+1)};
    Mat4 m = mat4Identity();
    Vec2f result = mat4MultiplyVec2(&v, &m);
    (void)result; // Prevent optimization
}

void benchmark_mat4_multiply_vec3(int i) {
    Vec3f v = {(float)i, (float)(i+1), (float)(i+2)};
    Mat4 m = mat4Identity();
    Vec3f result = mat4MultiplyVec3(&v, &m);
    (void)result; // Prevent optimization
}

void benchmark_mat4_multiply_vec4(int i) {
    Vec4f v = {(float)i, (float)(i+1), (float)(i+2), (float)(i+3)};
    Mat4 m = mat4Identity();
    Vec4f result = mat4MultiplyVec4(&v, &m);
    (void)result; // Prevent optimization
}

void benchmark_mat4_multiply_mat(int i) {
    Mat4 m1 = mat4Identity();
    Mat4 m2 = mat4Translate((Vec3f){(float)i, (float)(i+1), (float)(i+2)});
    Mat4 result = mat4MultiplyM(&m1, &m2);
    (void)result; // Prevent optimization
}

void benchmark_mat4_perspective(int i) {
    Mat4 result = mat4Perspective(0.1f, 100.0f, 1.0f, 45.0f);
    (void)result; // Prevent optimization
}

void benchmark_mat4_inverse(int i) {
    Mat4 m = mat4Scale((Vec3f){(float)i, (float)(i+1), (float)(i+2)});
    Mat4 result = mat4Inverse(&m);
    (void)result; // Prevent optimization
}

void benchmark_mat4() {
    printf("--- Mat4 Benchmarks ---\n");
    
    const int iterations = 1000000;
    
    benchmark_run("mat4Identity", iterations, benchmark_mat4_identity);
    benchmark_run("mat4Translate", iterations, benchmark_mat4_translate);
    benchmark_run("mat4RotateX", iterations, benchmark_mat4_rotateX);
    benchmark_run("mat4RotateY", iterations, benchmark_mat4_rotateY);
    benchmark_run("mat4RotateZ", iterations, benchmark_mat4_rotateZ);
    benchmark_run("mat4Scale", iterations, benchmark_mat4_scale);
    benchmark_run("mat4MultiplyVec2", iterations, benchmark_mat4_multiply_vec2);
    benchmark_run("mat4MultiplyVec3", iterations, benchmark_mat4_multiply_vec3);
    benchmark_run("mat4MultiplyVec4", iterations, benchmark_mat4_multiply_vec4);
    benchmark_run("mat4MultiplyM", iterations, benchmark_mat4_multiply_mat);
    benchmark_run("mat4Perspective", iterations, benchmark_mat4_perspective);
    benchmark_run("mat4Inverse", iterations, benchmark_mat4_inverse);
    
    printf("\n");
}
