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

#ifdef __cplusplus
}
#endif
