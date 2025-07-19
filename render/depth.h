#pragma once

#include <stdbool.h>
#include <stdint.h>

#define ZBUFFER32 // [ZBUFFER32 | ZBUFFER16 | ZBUFFER8]

#ifdef ZBUFFER32
typedef struct PingoDepth {
  uint32_t d;
} PingoDepth;
#endif

#ifdef ZBUFFER16
typedef struct PingoDepth {
  uint16_t d;
} PingoDepth;
#endif

#ifdef ZBUFFER8
typedef struct PingoDepth {
  uint8_t d;
} PingoDepth;
#endif

void depth_write(PingoDepth *d, int idx, float value);
bool depth_check(PingoDepth *d, int idx, float value);
