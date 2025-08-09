#include "test_math.h"
#include "../math/vec4.h"

int test_vec4() {
    printf("Testing Vec4 functions...\n");
    
    // Test Vec4f structure
    Vec4f v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    TEST_ASSERT_FLOAT_EQ(1.0f, v1.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, v1.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, v1.z, 0.001f);
    TEST_ASSERT_FLOAT_EQ(4.0f, v1.w, 0.001f);
    
    // Test Vec4i structure
    Vec4i v2 = {1, 2, 3, 4};
    TEST_ASSERT(v2.x == 1, "Vec4i x component failed");
    TEST_ASSERT(v2.y == 2, "Vec4i y component failed");
    TEST_ASSERT(v2.z == 3, "Vec4i z component failed");
    TEST_ASSERT(v2.w == 4, "Vec4i w component failed");
    
    // Test edge cases
    Vec4f zero = {0.0f, 0.0f, 0.0f, 0.0f};
    TEST_ASSERT_FLOAT_EQ(0.0f, zero.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, zero.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, zero.z, 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, zero.w, 0.001f);
    
    Vec4f negative = {-1.0f, -2.0f, -3.0f, -4.0f};
    TEST_ASSERT_FLOAT_EQ(-1.0f, negative.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(-2.0f, negative.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(-3.0f, negative.z, 0.001f);
    TEST_ASSERT_FLOAT_EQ(-4.0f, negative.w, 0.001f);
    
    printf("Vec4 tests completed successfully\n");
    return 1;
}
