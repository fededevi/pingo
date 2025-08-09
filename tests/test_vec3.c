#include "test_math.h"
#include "../math/vec3.h"

int test_vec3() {
    printf("Testing Vec3 functions...\n");
    
    // Test vec3f constructor
    Vec3f v1 = vec3f(1.0f, 2.0f, 3.0f);
    TEST_ASSERT_FLOAT_EQ(1.0f, v1.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, v1.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, v1.z, 0.001f);
    
    // Test vec3fmul (scalar multiplication)
    Vec3f v2 = vec3fmul(v1, 2.0f);
    TEST_ASSERT_FLOAT_EQ(2.0f, v2.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(4.0f, v2.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(6.0f, v2.z, 0.001f);
    
    // Test vec3fsumV (vector addition)
    Vec3f v3 = vec3f(1.0f, 1.0f, 1.0f);
    Vec3f sum = vec3fsumV(v1, v3);
    TEST_ASSERT_FLOAT_EQ(2.0f, sum.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, sum.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(4.0f, sum.z, 0.001f);
    
    // Test vec3fsubV (vector subtraction)
    Vec3f diff = vec3fsubV(v1, v3);
    TEST_ASSERT_FLOAT_EQ(0.0f, diff.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, diff.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, diff.z, 0.001f);
    
    // Test vec3fsum (scalar addition)
    Vec3f v4 = vec3fsum(v1, 5.0f);
    TEST_ASSERT_FLOAT_EQ(6.0f, v4.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(7.0f, v4.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(8.0f, v4.z, 0.001f);
    
    // Test vec3Dot (dot product)
    Vec3f a = vec3f(1.0f, 0.0f, 0.0f);
    Vec3f b = vec3f(0.0f, 1.0f, 0.0f);
    F_TYPE dot_perpendicular = vec3Dot(a, b);
    TEST_ASSERT_FLOAT_EQ(0.0f, dot_perpendicular, 0.001f);
    
    Vec3f c = vec3f(1.0f, 1.0f, 1.0f);
    F_TYPE dot_parallel = vec3Dot(c, c);
    TEST_ASSERT_FLOAT_EQ(3.0f, dot_parallel, 0.001f);
    
    // Test vec3Cross (cross product)
    Vec3f cross = vec3Cross(a, b);
    TEST_ASSERT_FLOAT_EQ(0.0f, cross.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, cross.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, cross.z, 0.001f);
    
    // Test vec3Normalize
    Vec3f long_vec = vec3f(3.0f, 4.0f, 0.0f);
    Vec3f normalized = vec3Normalize(long_vec);
    F_TYPE length = sqrtf(normalized.x * normalized.x + 
                          normalized.y * normalized.y + 
                          normalized.z * normalized.z);
    TEST_ASSERT_FLOAT_EQ(1.0f, length, 0.001f);
    
    // Test edge cases
    Vec3f zero = vec3f(0.0f, 0.0f, 0.0f);
    Vec3f zero_sum = vec3fsumV(v1, zero);
    TEST_ASSERT_VEC3F_EQ(v1, zero_sum, 0.001f);
    
    Vec3f zero_mul = vec3fmul(zero, 5.0f);
    TEST_ASSERT_VEC3F_EQ(zero, zero_mul, 0.001f);
    
    // Test normalization of zero vector (should handle gracefully)
    Vec3f zero_norm = vec3Normalize(zero);
    // This might return zero or handle the case specially
    
    printf("Vec3 tests completed successfully\n");
    return 1;
}
