#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vec3i {
  I_TYPE x;
  I_TYPE y;
  I_TYPE z;
} Vec3i;

typedef struct Vec3f {
  F_TYPE x;
  F_TYPE y;
  F_TYPE z;
} Vec3f;

// Defined here rather than in vec3.c on purpose. Each is a handful of
// multiplies, and the renderer calls several of them per triangle; out of line
// they are calls the compiler cannot see through unless link-time optimization
// is both available and building statically. In a header they are inlined
// always. Measured at 11% of the render time on a shared build without LTO.
#include <math.h>

static inline Vec3f vec3f(F_TYPE x, F_TYPE y, F_TYPE z) {
  return (Vec3f){x, y, z};
}

static inline Vec3f vec3fmul(Vec3f a, F_TYPE b) {
  return (Vec3f){a.x * b, a.y * b, a.z * b};
}

static inline Vec3f vec3fsumV(Vec3f a, Vec3f b) {
  return (Vec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3f vec3fsubV(Vec3f a, Vec3f b) {
  return (Vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3f vec3fsum(Vec3f a, F_TYPE b) {
  return (Vec3f){a.x + b, a.y + b, a.z + b};
}

static inline F_TYPE vec3Dot(Vec3f a, Vec3f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3f vec3Cross(Vec3f a, Vec3f b) {
  return (Vec3f){a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x,
                 a.x * b.y - b.x * a.y};
}

static inline Vec3f vec3Normalize(Vec3f v) {
  const F_TYPE length_sq = v.x * v.x + v.y * v.y + v.z * v.z;
  if (length_sq == 0.0f) {
    return (Vec3f){0.0f, 0.0f, 0.0f};
  }
  // Already unit length: the common case for a light direction.
  if (length_sq == 1.0f) {
    return v;
  }
  const F_TYPE inv_length = 1.0f / sqrtf(length_sq);
  return (Vec3f){v.x * inv_length, v.y * inv_length, v.z * inv_length};
}

#ifdef __cplusplus
}
#endif
