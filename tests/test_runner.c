#include "test_math.h"

// Test runner
int run_all_tests() {
    printf("Running math library tests...\n\n");
    
    int passed = 0;
    int total = 0;
    
    #define RUN_TEST(test_func) \
        do { \
            printf("Running " #test_func "...\n"); \
            if (test_func()) { \
                printf("PASS: " #test_func "\n"); \
                passed++; \
            } else { \
                printf("FAIL: " #test_func "\n"); \
            } \
            total++; \
            printf("\n"); \
        } while(0)
    
    RUN_TEST(test_vec2);
    RUN_TEST(test_vec3);
    RUN_TEST(test_vec4);
    RUN_TEST(test_mat3);
    RUN_TEST(test_mat4);
    RUN_TEST(test_functions);
    
    printf("Test Results: %d/%d tests passed\n", passed, total);
    return passed == total;
}
