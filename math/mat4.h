#pragma once

#include "types.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

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

Vec2f mat4MultiplyVec2(const Vec2f *v, const Mat4 *t);
Vec3f mat4MultiplyVec3(const Vec3f *v, const Mat4 *t);

// Six calls per triangle, sixteen multiplies each: the one matrix routine
// worth inlining unconditionally.
static inline Vec4f mat4MultiplyVec4(const Vec4f *v, const Mat4 *t) {
  const F_TYPE *e = t->elements;
  return (Vec4f){
      v->x * e[0] + v->y * e[1] + v->z * e[2] + v->w * e[3],
      v->x * e[4] + v->y * e[5] + v->z * e[6] + v->w * e[7],
      v->x * e[8] + v->y * e[9] + v->z * e[10] + v->w * e[11],
      v->x * e[12] + v->y * e[13] + v->z * e[14] + v->w * e[15]};
}

Mat4 mat4MultiplyM(const Mat4 *m1, const Mat4 *m2);
F_TYPE mat4Determinant(const Mat4 *mat);
Mat4 mat4Inverse(const Mat4 *mat);
Mat4 mat4Scale(Vec3f s);

Mat4 mat4Perspective(F_TYPE near, F_TYPE far, F_TYPE aspect, F_TYPE fov);

F_TYPE mat4NearFromProjection(Mat4 mat);
F_TYPE mat4FarFromProjection(Mat4 mat);

#ifdef __cplusplus
}
#endif
