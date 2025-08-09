#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Test utilities
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s at %s:%d\n", message, __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_FLOAT_EQ(expected, actual, tolerance) \
    do { \
        if (fabs((expected) - (actual)) > (tolerance)) { \
            printf("FAIL: Expected %f, got %f at %s:%d\n", (expected), (actual), __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_VEC2F_EQ(expected, actual, tolerance) \
    do { \
        if (fabs((expected).x - (actual).x) > (tolerance) || \
            fabs((expected).y - (actual).y) > (tolerance)) { \
            printf("FAIL: Expected Vec2f(%f, %f), got Vec2f(%f, %f) at %s:%d\n", \
                   (expected).x, (expected).y, (actual).x, (actual).y, __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_VEC3F_EQ(expected, actual, tolerance) \
    do { \
        if (fabs((expected).x - (actual).x) > (tolerance) || \
            fabs((expected).y - (actual).y) > (tolerance) || \
            fabs((expected).z - (actual).z) > (tolerance)) { \
            printf("FAIL: Expected Vec3f(%f, %f, %f), got Vec3f(%f, %f, %f) at %s:%d\n", \
                   (expected).x, (expected).y, (expected).z, \
                   (actual).x, (actual).y, (actual).z, __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_MAT3_EQ(expected, actual, tolerance) \
    do { \
        for (int i = 0; i < 9; i++) { \
            if (fabs((expected).elements[i] - (actual).elements[i]) > (tolerance)) { \
                printf("FAIL: Mat3 elements[%d] expected %f, got %f at %s:%d\n", \
                       i, (expected).elements[i], (actual).elements[i], __FILE__, __LINE__); \
                return 0; \
            } \
        } \
    } while(0)

#define TEST_ASSERT_MAT4_EQ(expected, actual, tolerance) \
    do { \
        for (int i = 0; i < 16; i++) { \
            if (fabs((expected).elements[i] - (actual).elements[i]) > (tolerance)) { \
                printf("FAIL: Mat4 elements[%d] expected %f, got %f at %s:%d\n", \
                       i, (expected).elements[i], (actual).elements[i], __FILE__, __LINE__); \
                return 0; \
            } \
        } \
    } while(0)

// Test function prototypes
int test_vec2();
int test_vec3();
int test_vec4();
int test_mat3();
int test_mat4();
int test_functions();
