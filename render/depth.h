#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ZBUFFER32 // [ZBUFFER32 | ZBUFFER16 | ZBUFFER8]

#ifdef ZBUFFER32
typedef struct PingoDepth {
    uint32_t d;
} PingoDepth;
#endif

#ifdef ZBUFFER16
typedef struct Depth {
    uint16_t d;
} Depth;
#endif

#ifdef ZBUFFER8
typedef struct Depth {
    uint8_t d;
} Depth;
#endif

void depth_write(PingoDepth * d, int idx, float value);
bool depth_check(PingoDepth * d, int idx, float value);

