#include "test_math.h"
#include "../math/vec2.h"

int test_vec2() {
    printf("Testing Vec2 functions...\n");
    
    // Test Vec2i operations
    Vec2i v1 = {1, 2};
    Vec2i v2 = {3, 4};
    
    // Test vector2ISum
    Vec2i sum = vector2ISum(v1, v2);
    TEST_ASSERT(sum.x == 4 && sum.y == 6, "vector2ISum failed");
    
    // Test vecItoF conversion
    Vec2f vf = vecItoF(v1);
    TEST_ASSERT_FLOAT_EQ(1.0f, vf.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, vf.y, 0.001f);
    
    // Test vecFtoI conversion
    Vec2f vf2 = {5.7f, 8.9f};
    Vec2i vi = vecFtoI(vf2);
    TEST_ASSERT(vi.x == 5 && vi.y == 8, "vecFtoI failed");
    
    // Test edge cases
    Vec2i zero = {0, 0};
    Vec2i negative = {-1, -2};
    
    Vec2i sum_zero = vector2ISum(v1, zero);
    TEST_ASSERT(sum_zero.x == v1.x && sum_zero.y == v1.y, "Adding zero vector failed");
    
    Vec2i sum_negative = vector2ISum(v1, negative);
    TEST_ASSERT(sum_negative.x == 0 && sum_negative.y == 0, "Adding negative vector failed");
    
    // Test float conversion edge cases
    Vec2f zero_f = vecItoF(zero);
    TEST_ASSERT_FLOAT_EQ(0.0f, zero_f.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, zero_f.y, 0.001f);
    
    Vec2i negative_conv = vecFtoI(vecItoF(negative));
    TEST_ASSERT(negative_conv.x == negative.x && negative_conv.y == negative.y, 
                "Negative conversion failed");
    
    printf("Vec2 tests completed successfully\n");
    return 1;
}
