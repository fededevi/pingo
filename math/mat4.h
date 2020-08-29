#pragma once

#include "types.h"
#include "vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mat4 {
    F_TYPE elements[16];
} Mat4;

Mat4 mat4Identity();
Mat4 mat4Translate(Vec3f l);

Mat4 mat4RotateX(F_TYPE phi);
Mat4 mat4RotateY(F_TYPE phi);
Mat4 mat4RotateZ(F_TYPE phi);

Mat4 mat4MultiplyM( Mat4 * m1, Mat4 * m2);
Mat4 mat4Inverse(Mat4 * mat);
Mat4 mat4Scale(Vec3f s);

#ifdef __cplusplus
}
#endif
