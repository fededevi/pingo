#include "test_math.h"
#include "../math/mat3.h"

int test_mat3() {
    printf("Testing Mat3 functions...\n");
    
    // Test mat3Identity
    Mat3 identity = mat3Identity();
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, identity.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, identity.elements[8], 0.001f);
    
    // Test mat3Translate
    Vec2f trans_vec = {2.0f, 3.0f};
    Mat3 translate = mat3Translate(trans_vec);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, translate.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, translate.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, translate.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, translate.elements[8], 0.001f);
    
    // Test mat3Rotate
    Mat3 rotate_90 = mat3Rotate(M_PI / 2.0f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_90.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(-1.0f, rotate_90.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_90.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_90.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_90.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_90.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_90.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, rotate_90.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, rotate_90.elements[8], 0.001f);
    
    // Test mat3Scale
    Vec2f scale_vec = {2.0f, 3.0f};
    Mat3 scale = mat3Scale(scale_vec);
    TEST_ASSERT_FLOAT_EQ(2.0f, scale.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, scale.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[5], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[6], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, scale.elements[7], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, scale.elements[8], 0.001f);
    
    // Test mat3Multiply (vector multiplication)
    Vec2f test_vec = {1.0f, 2.0f};
    Vec2f result = mat3Multiply(&test_vec, &identity);
    TEST_ASSERT_FLOAT_EQ(1.0f, result.x, 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, result.y, 0.001f);
    
    // Test mat3MultiplyM (matrix multiplication)
    Mat3 result_mat = mat3MultiplyM(&identity, &translate);
    TEST_ASSERT_MAT3_EQ(translate, result_mat, 0.001f);
    
    // Test mat3Determinant
    F_TYPE det_identity = mat3Determinant(&identity);
    TEST_ASSERT_FLOAT_EQ(1.0f, det_identity, 0.001f);
    
    F_TYPE det_scale = mat3Determinant(&scale);
    TEST_ASSERT_FLOAT_EQ(6.0f, det_scale, 0.001f);
    
    // Test mat3Inverse
    Mat3 inv_scale = mat3Inverse(&scale);
    Mat3 identity_check = mat3MultiplyM(&scale, &inv_scale);
    TEST_ASSERT_MAT3_EQ(identity, identity_check, 0.001f);
    
    // Test mat3IsOnlyTranslation
    int is_only_trans = mat3IsOnlyTranslation(&translate);
    TEST_ASSERT(is_only_trans == 1, "mat3IsOnlyTranslation failed for translation matrix");
    
    int is_only_trans_identity = mat3IsOnlyTranslation(&identity);
    TEST_ASSERT(is_only_trans_identity == 1, "mat3IsOnlyTranslation failed for identity matrix");
    
    int is_only_trans_scale = mat3IsOnlyTranslation(&scale);
    TEST_ASSERT(is_only_trans_scale == 0, "mat3IsOnlyTranslation failed for scale matrix");
    
    // Test mat3Complete
    Vec2f origin = {0.0f, 0.0f};
    Vec2f translation = {1.0f, 2.0f};
    Vec2f scale_complete = {2.0f, 2.0f};
    F_TYPE rotation = 0.0f;
    Mat3 complete = mat3Complete(origin, translation, scale_complete, rotation);
    
    // The complete matrix should have scale and translation
    TEST_ASSERT_FLOAT_EQ(2.0f, complete.elements[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, complete.elements[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, complete.elements[2], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, complete.elements[3], 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, complete.elements[4], 0.001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, complete.elements[5], 0.001f);
    
    printf("Mat3 tests completed successfully\n");
    return 1;
}
