#include "test_math.h"
#include "../math/mat4.h"

int test_mat4() {
    printf("Testing Mat4 functions...\n");
    
    // Test mat4Identity
    Mat4 identity = mat4Identity();
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[10], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[11], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[12], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[13], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[14], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[15], 0.001f);
    
    // Test mat4Translate
    Vec3f trans_vec = {1.0f, 2.0f, 3.0f};
    Mat4 translate = mat4Translate(trans_vec);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, translate.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[10], 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, translate.elements[11], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[12], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[13], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[14], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[15], 0.001f);
    
    // Test mat4RotateX
    Mat4 rotate_x = mat4RotateX(M_PI / 2.0f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_x.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(-1.0f, rotate_x.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_x.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[10], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_x.elements[11], 0.001f);
    
    // Test mat4RotateY
    Mat4 rotate_y = mat4RotateY(M_PI / 2.0f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_y.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_y.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(-1.0f, rotate_y.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[10], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_y.elements[11], 0.001f);
    
    // Test mat4RotateZ
    Mat4 rotate_z = mat4RotateZ(M_PI / 2.0f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(-1.0f, rotate_z.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_z.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_z.elements[10], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_z.elements[11], 0.001f);
    
    // Test mat4Scale
    Vec3f scale_vec = {2.0f, 3.0f, 4.0f};
    Mat4 scale = mat4Scale(scale_vec);
    TEST_ASSERT_FLOAT_EQ(2.0f, scale.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, scale.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(4.0f, scale.elements[10], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[11], 0.001f);
    
    // Test mat4MultiplyVec2
    Vec2f test_vec2 = {1.0f, 2.0f};
    Vec2f result_vec2 = mat4MultiplyVec2(&test_vec2, &identity);
    TEST_ASSERT_FLOAT_EQ(1.0f, result_vec2.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, result_vec2.y, 0.001f);
    
    // Test mat4MultiplyVec3
    Vec3f test_vec3 = {1.0f, 2.0f, 3.0f};
    Vec3f result_vec3 = mat4MultiplyVec3(&test_vec3, &identity);
    TEST_ASSERT_FLOAT_EQ(1.0f, result_vec3.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, result_vec3.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, result_vec3.z, 0.001f);
    
    // Test mat4MultiplyVec4
    Vec4f test_vec4 = {1.0f, 2.0f, 3.0f, 4.0f};
    Vec4f result_vec4 = mat4MultiplyVec4(&test_vec4, &identity);
    TEST_ASSERT_FLOAT_EQ(1.0f, result_vec4.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, result_vec4.y, 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, result_vec4.z, 0.001f);
    TEST_ASSERT_FLOAT_EQ(4.0f, result_vec4.w, 0.001f);
    
    // Test mat4MultiplyM
    Mat4 result_mat = mat4MultiplyM(&identity, &translate);
    TEST_ASSERT_MAT4_EQ(translate, result_mat, 0.001f);
    
    // Test mat4Perspective
    Mat4 perspective = mat4Perspective(0.1f, 100.0f, 1.0f, 45.0f);
    // Perspective matrix should have specific properties
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[8], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[9], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[12], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, perspective.elements[13], 0.001f);
    TEST_ASSERT_FLOAT_EQ(-1.0f, perspective.elements[15], 0.001f);
    
    // Test mat4Inverse
    Mat4 inv_scale = mat4Inverse(&scale);
    Mat4 identity_check = mat4MultiplyM(&scale, &inv_scale);
    TEST_ASSERT_MAT4_EQ(identity, identity_check, 0.001f);
    
    // Test mat4NearFromProjection and mat4FarFromProjection
    F_TYPE near_val = mat4NearFromProjection(perspective);
    F_TYPE far_val = mat4FarFromProjection(perspective);
    TEST_ASSERT_FLOAT_EQ(0.1f, near_val, 0.001f);
    TEST_ASSERT_FLOAT_EQ(100.0f, far_val, 0.001f);
    
    printf("Mat4 tests completed successfully\n");
    return 1;
}
