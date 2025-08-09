#include "test_math.h"
#include "../math/fun.h"

int test_functions() {
    printf("Testing mathematical utility functions...\n");
    
    // Test MIN and MAX macros
    TEST_ASSERT(MIN(5, 3) == 3, "MIN macro failed");
    TEST_ASSERT(MIN(3, 5) == 3, "MIN macro failed");
    TEST_ASSERT(MIN(-5, 3) == -5, "MIN macro failed");
    TEST_ASSERT(MIN(3, -5) == -5, "MIN macro failed");
    
    TEST_ASSERT(MAX(5, 3) == 5, "MAX macro failed");
    TEST_ASSERT(MAX(3, 5) == 5, "MAX macro failed");
    TEST_ASSERT(MAX(-5, 3) == 3, "MAX macro failed");
    TEST_ASSERT(MAX(3, -5) == 3, "MAX macro failed");
    
    // Test edgeFunction
    Vec2f a = {0.0f, 0.0f};
    Vec2f b = {1.0f, 0.0f};
    Vec2f c = {0.0f, 1.0f};
    
    int edge_result = edgeFunction(&a, &b, &c);
    TEST_ASSERT(edge_result < 0, "edgeFunction failed for counter-clockwise triangle");
    
    // Test with clockwise triangle
    Vec2f d = {1.0f, 1.0f};
    int edge_result2 = edgeFunction(&a, &b, &d);
    TEST_ASSERT(edge_result2 < 0, "edgeFunction failed for clockwise triangle");
    
    // Test with collinear points
    Vec2f e = {2.0f, 0.0f};
    int edge_result3 = edgeFunction(&a, &b, &e);
    TEST_ASSERT(edge_result3 == 0, "edgeFunction failed for collinear points");
    
    // Test isClockWise
    float clockwise_result = isClockWise(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    TEST_ASSERT(clockwise_result < 0, "isClockWise failed for counter-clockwise triangle");
    
    float clockwise_result2 = isClockWise(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
    TEST_ASSERT(clockwise_result2 < 0, "isClockWise failed for clockwise triangle");
    
    // Test orient2d
    Vec2i a_int = {0, 0};
    Vec2i b_int = {1, 0};
    Vec2i c_int = {0, 1};
    
    int orient_result = orient2d(a_int, b_int, c_int);
    TEST_ASSERT(orient_result > 0, "orient2d failed for counter-clockwise triangle");
    
    Vec2i d_int = {1, 1};
    int orient_result2 = orient2d(a_int, b_int, d_int);
    TEST_ASSERT(orient_result2 > 0, "orient2d failed for counter-clockwise triangle");
    
    Vec2i e_int = {2, 0};
    int orient_result3 = orient2d(a_int, b_int, e_int);
    TEST_ASSERT(orient_result3 == 0, "orient2d failed for collinear points");
    
    printf("Mathematical utility function tests completed successfully\n");
    return 1;
}
