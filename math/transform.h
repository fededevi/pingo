#pragma once

#include "vector2I.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float T;

typedef struct {
    T elements[9];
} Transform;

extern Transform transformTranslate(Vector2I l);
extern Transform transformRotate(float theta);

extern Vector2I transformMultiply(Vector2I * v, Transform * t);

#ifdef __cplusplus
}
#endif
