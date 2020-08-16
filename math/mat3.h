#pragma once

#include "vec2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float T;

typedef struct {
    T elements[9];
} Mat3;

extern Mat3 transformTranslate(Vec2f l);
extern Mat3 transformRotate(float theta);
extern Mat3 transformScale(Vec2f s);

extern Vec2f transformMultiply(Vec2f * v, Mat3 * t);
extern Mat3  transformMultiplyM( Mat3 *v, Mat3 *t);

extern Mat3 mat3Inverse( Mat3 *v );

extern T mat3Determinant(Mat3 * m);
extern int transformIsOnlyTranslation(Mat3 *m);

#ifdef __cplusplus
}
#endif
