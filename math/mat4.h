#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mat3 {
    F_TYPE elements[16];
} Mat3;

#ifdef __cplusplus
}
#endif
