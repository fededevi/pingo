#pragma once

#include "fwd.h"

#include <stdbool.h>
#include <stdint.h>

#define ZBUFFER32 // [ZBUFFER32 | ZBUFFER16 | ZBUFFER8]

#ifdef ZBUFFER32
struct PingoDepth {
  uint32_t d;
};
#endif

#ifdef ZBUFFER16
struct PingoDepth {
  uint16_t d;
};
#endif

#ifdef ZBUFFER8
struct PingoDepth {
  uint8_t d;
};
#endif

void depth_write(PingoDepth *d, int idx, float value);
bool depth_check(PingoDepth *d, int idx, float value);
