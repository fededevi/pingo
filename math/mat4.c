#include "mat4.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include <float.h>
#include <math.h>
#include <stdint.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

Mat4 mat4Identity() {
  return (Mat4){{
      1,
      0,
      0,
      0,
      0,
      1,
      0,
      0,
      0,
      0,
      1,
      0,
      0,
      0,
      0,
      1,
  }};
}

Mat4 mat4Translate(Vec3f l) {
  F_TYPE x = l.x;
  F_TYPE y = l.y;
  F_TYPE z = l.z;
  return (Mat4){{
      1,
      0,
      0,
      x,
      0,
      1,
      0,
      y,
      0,
      0,
      1,
      z,
      0,
      0,
      0,
      1,
  }};
}

Mat4 mat4RotateX(F_TYPE phi) {
  F_TYPE s = sin(phi);
  F_TYPE c = cos(phi);
  return (Mat4){{
      1,
      0,
      0,
      0,
      0,
      c,
      -s,
      0,
      0,
      s,
      c,
      0,
      0,
      0,
      0,
      1,
  }};
}
Mat4 mat4RotateY(F_TYPE phi) {
  F_TYPE s = sin(phi);
  F_TYPE c = cos(phi);
  return (Mat4){{
      c,
      0,
      s,
      0,
      0,
      1,
      0,
      0,
      -s,
      0,
      c,
      0,
      0,
      0,
      0,
      1,
  }};
}
Mat4 mat4RotateZ(F_TYPE phi) {
  F_TYPE s = sin(phi);
  F_TYPE c = cos(phi);
  return (Mat4){{
      c,
      -s,
      0,
      0,
      s,
      c,
      0,
      0,
      0,
      0,
      1,
      0,
      0,
      0,
      0,
      1,
  }};
}

extern Mat4 mat4Scale(Vec3f s) {
  F_TYPE p = s.x;
  F_TYPE q = s.y;
  F_TYPE r = s.z;
  return (Mat4){{
      p,
      0,
      0,
      0,
      0,
      q,
      0,
      0,
      0,
      0,
      r,
      0,
      0,
      0,
      0,
      1,
  }};
}

Vec2f mat4MultiplyVec2(const Vec2f *v, const Mat4 *t) {
  F_TYPE a = v->x * t->elements[0] + v->y * t->elements[1] +
             1.0 * t->elements[2] + 1.0 * t->elements[3];
  F_TYPE b = v->x * t->elements[4] + v->y * t->elements[5] +
             1.0 * t->elements[6] + 1.0 * t->elements[7];
  return (Vec2f){a, b};
}

Vec3f mat4MultiplyVec3(const Vec3f *v, const Mat4 *t) {
  F_TYPE a = v->x * t->elements[0] + v->y * t->elements[1] +
             v->z * t->elements[2] + 1.0 * t->elements[3];
  F_TYPE b = v->x * t->elements[4] + v->y * t->elements[5] +
             v->z * t->elements[6] + 1.0 * t->elements[7];
  F_TYPE c = v->x * t->elements[8] + v->y * t->elements[9] +
             v->z * t->elements[10] + 1.0 * t->elements[11];
  return (Vec3f){a, b, c};
}

Mat4 mat4MultiplyM(const Mat4 *m1, const Mat4 *m2) {
  const F_TYPE *a = m2->elements;
  const F_TYPE *b = m1->elements;

  // Fast path for identity matrix multiplication
  if (a[0] == 1.0f && a[1] == 0.0f && a[2] == 0.0f && a[3] == 0.0f &&
      a[4] == 0.0f && a[5] == 1.0f && a[6] == 0.0f && a[7] == 0.0f &&
      a[8] == 0.0f && a[9] == 0.0f && a[10] == 1.0f && a[11] == 0.0f &&
      a[12] == 0.0f && a[13] == 0.0f && a[14] == 0.0f && a[15] == 1.0f) {
    return *m1; // Identity * matrix = matrix
  }

  if (b[0] == 1.0f && b[1] == 0.0f && b[2] == 0.0f && b[3] == 0.0f &&
      b[4] == 0.0f && b[5] == 1.0f && b[6] == 0.0f && b[7] == 0.0f &&
      b[8] == 0.0f && b[9] == 0.0f && b[10] == 1.0f && b[11] == 0.0f &&
      b[12] == 0.0f && b[13] == 0.0f && b[14] == 0.0f && b[15] == 1.0f) {
    return *m2; // matrix * Identity = matrix
  }

  // Fast path for translation matrix multiplication
  if (a[0] == 1.0f && a[1] == 0.0f && a[2] == 0.0f && a[4] == 0.0f &&
      a[5] == 1.0f && a[6] == 0.0f && a[8] == 0.0f && a[9] == 0.0f &&
      a[10] == 1.0f && a[12] == 0.0f && a[13] == 0.0f && a[14] == 0.0f &&
      a[15] == 1.0f && b[0] == 1.0f && b[1] == 0.0f && b[2] == 0.0f &&
      b[4] == 0.0f && b[5] == 1.0f && b[6] == 0.0f && b[8] == 0.0f &&
      b[9] == 0.0f && b[10] == 1.0f && b[12] == 0.0f && b[13] == 0.0f &&
      b[14] == 0.0f && b[15] == 1.0f) {
    // Translation * Translation = combined translation
    return (Mat4){{1.0f, 0.0f, 0.0f, a[3] + b[3], 0.0f, 1.0f, 0.0f, a[7] + b[7],
                   0.0f, 0.0f, 1.0f, a[11] + b[11], 0.0f, 0.0f, 0.0f, 1.0f}};
  }

  // General case
  Mat4 out;

  out.elements[0x0] =
      a[0x0] * b[0x0] + a[0x1] * b[0x4] + a[0x2] * b[0x8] + a[0x3] * b[0xc];
  out.elements[0x1] =
      a[0x0] * b[0x1] + a[0x1] * b[0x5] + a[0x2] * b[0x9] + a[0x3] * b[0xd];
  out.elements[0x2] =
      a[0x0] * b[0x2] + a[0x1] * b[0x6] + a[0x2] * b[0xa] + a[0x3] * b[0xe];
  out.elements[0x3] =
      a[0x0] * b[0x3] + a[0x1] * b[0x7] + a[0x2] * b[0xb] + a[0x3] * b[0xf];

  out.elements[0x4] =
      a[0x4] * b[0x0] + a[0x5] * b[0x4] + a[0x6] * b[0x8] + a[0x7] * b[0xc];
  out.elements[0x5] =
      a[0x4] * b[0x1] + a[0x5] * b[0x5] + a[0x6] * b[0x9] + a[0x7] * b[0xd];
  out.elements[0x6] =
      a[0x4] * b[0x2] + a[0x5] * b[0x6] + a[0x6] * b[0xa] + a[0x7] * b[0xe];
  out.elements[0x7] =
      a[0x4] * b[0x3] + a[0x5] * b[0x7] + a[0x6] * b[0xb] + a[0x7] * b[0xf];

  out.elements[0x8] =
      a[0x8] * b[0x0] + a[0x9] * b[0x4] + a[0xa] * b[0x8] + a[0xb] * b[0xc];
  out.elements[0x9] =
      a[0x8] * b[0x1] + a[0x9] * b[0x5] + a[0xa] * b[0x9] + a[0xb] * b[0xd];
  out.elements[0xA] =
      a[0x8] * b[0x2] + a[0x9] * b[0x6] + a[0xa] * b[0xa] + a[0xb] * b[0xe];
  out.elements[0xB] =
      a[0x8] * b[0x3] + a[0x9] * b[0x7] + a[0xa] * b[0xb] + a[0xb] * b[0xf];

  out.elements[0xC] =
      a[0xc] * b[0x0] + a[0xd] * b[0x4] + a[0xe] * b[0x8] + a[0xf] * b[0xc];
  out.elements[0xD] =
      a[0xc] * b[0x1] + a[0xd] * b[0x5] + a[0xe] * b[0x9] + a[0xf] * b[0xd];
  out.elements[0xE] =
      a[0xc] * b[0x2] + a[0xd] * b[0x6] + a[0xe] * b[0xa] + a[0xf] * b[0xe];
  out.elements[0xF] =
      a[0xc] * b[0x3] + a[0xd] * b[0x7] + a[0xe] * b[0xb] + a[0xf] * b[0xf];

  return out;
}

F_TYPE mat4Determinant(const Mat4 *mat) {
  const F_TYPE *a = mat->elements;
  F_TYPE a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3], a10 = a[4], a11 = a[5],
         a12 = a[6], a13 = a[7], a20 = a[8], a21 = a[9], a22 = a[10],
         a23 = a[11], a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

  F_TYPE b00 = a00 * a11 - a01 * a10;
  F_TYPE b01 = a00 * a12 - a02 * a10;
  F_TYPE b02 = a00 * a13 - a03 * a10;
  F_TYPE b03 = a01 * a12 - a02 * a11;
  F_TYPE b04 = a01 * a13 - a03 * a11;
  F_TYPE b05 = a02 * a13 - a03 * a12;
  F_TYPE b06 = a20 * a31 - a21 * a30;
  F_TYPE b07 = a20 * a32 - a22 * a30;
  F_TYPE b08 = a20 * a33 - a23 * a30;
  F_TYPE b09 = a21 * a32 - a22 * a31;
  F_TYPE b10 = a21 * a33 - a23 * a31;
  F_TYPE b11 = a22 * a33 - a23 * a32;

  // Calculate the determinant
  return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
}

Mat4 mat4Inverse(const Mat4 *mat) {
  const F_TYPE *m = mat->elements;

  // Fast path for identity matrix
  if (m[0] == 1.0f && m[1] == 0.0f && m[2] == 0.0f && m[3] == 0.0f &&
      m[4] == 0.0f && m[5] == 1.0f && m[6] == 0.0f && m[7] == 0.0f &&
      m[8] == 0.0f && m[9] == 0.0f && m[10] == 1.0f && m[11] == 0.0f &&
      m[12] == 0.0f && m[13] == 0.0f && m[14] == 0.0f && m[15] == 1.0f) {
    return mat4Identity();
  }

  // Fast path for translation-only matrix
  if (m[0] == 1.0f && m[1] == 0.0f && m[2] == 0.0f && m[4] == 0.0f &&
      m[5] == 1.0f && m[6] == 0.0f && m[8] == 0.0f && m[9] == 0.0f &&
      m[10] == 1.0f && m[12] == 0.0f && m[13] == 0.0f && m[14] == 0.0f &&
      m[15] == 1.0f) {
    return (Mat4){{1.0f, 0.0f, 0.0f, -m[3], 0.0f, 1.0f, 0.0f, -m[7], 0.0f, 0.0f,
                   1.0f, -m[11], 0.0f, 0.0f, 0.0f, 1.0f}};
  }

  // Fast path for scale-only matrix
  if (m[1] == 0.0f && m[2] == 0.0f && m[3] == 0.0f && m[4] == 0.0f &&
      m[6] == 0.0f && m[7] == 0.0f && m[8] == 0.0f && m[9] == 0.0f &&
      m[11] == 0.0f && m[12] == 0.0f && m[13] == 0.0f && m[14] == 0.0f &&
      m[15] == 1.0f) {
    F_TYPE inv_x = 1.0f / m[0];
    F_TYPE inv_y = 1.0f / m[5];
    F_TYPE inv_z = 1.0f / m[10];
    return (Mat4){{inv_x, 0.0f, 0.0f, 0.0f, 0.0f, inv_y, 0.0f, 0.0f, 0.0f, 0.0f,
                   inv_z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};
  }

  // General case - use the original algorithm
  F_TYPE inv[16], det;

  inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
           m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

  inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
           m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

  inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
           m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

  inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
            m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

  inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
           m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

  inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
           m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

  inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
           m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

  inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
            m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

  inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
           m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

  inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
           m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

  inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

  inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
           m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
           m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  // Return identity matrix for singular matrices (det == 0) to avoid NaN
  // propagation
  if (det == 0.0) {
    return mat4Identity();
  }

  Mat4 out;
  det = 1.0 / det;

  for (int i = 0; i < 16; i++)
    out.elements[i] = inv[i] * det;

  return out;
}

Mat4 mat4Perspective(F_TYPE near, F_TYPE far, F_TYPE aspect, F_TYPE fovy) {
  F_TYPE h = 1.0 / tan(fovy * 0.5);
  F_TYPE w = 1.0 / tan(aspect * fovy * 0.5);
  F_TYPE x = ((far) / (far - near));
  F_TYPE y = (2 * far * near) / (far - near);

  Mat4 m = {{w, 0, 0, 0, 0, h, 0, 0, 0, 0, x, -1, 0, 0, -y, -1}};

  return m;
}

/* mat4NearFromProjection / mat4FarFromProjection invert the depth block of
 * mat4Perspective(), which stores
 *     elements[10] =  far / (far - near)              -> A
 *     elements[14] = -2 * far * near / (far - near)   -> C
 * Dividing the two gives C / A = -2 * near, hence near = -C / (2A).
 * Substituting that back into A yields far = -C / (2 * (A - 1)).
 * of two in elements[14]) and is not inverted by these functions. */
F_TYPE mat4NearFromProjection(Mat4 mat) {
  F_TYPE A = mat.elements[10];
  F_TYPE C = mat.elements[14];

  // A == 0 means the matrix carries no depth mapping to invert.
  if (A == 0)
    return 0;

  return -C / (2 * A);
}

F_TYPE mat4FarFromProjection(Mat4 mat) {
  F_TYPE A = mat.elements[10];
  F_TYPE C = mat.elements[14];

  // A tends to 1 as far tends to infinity, so A == 1 is an infinite far
  // plane. FLT_MAX stands in for it rather than INFINITY, which is a C99
  // math.h macro that the smaller embedded libcs do not all define - msp430's
  // does not, and this library is meant to build there.
  if (A == 1)
    return FLT_MAX;

  return -C / (2 * (A - 1));
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
