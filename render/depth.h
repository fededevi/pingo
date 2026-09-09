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

// Inline, and in one function rather than two. Called for every candidate
// pixel: as separate out-of-line calls this was two calls, two indexings of
// the buffer and - because each converted the float itself - the same
// float-to-integer conversion done twice.
#ifdef ZBUFFER32
typedef uint32_t PingoDepthValue;
#define PINGO_DEPTH_MAX UINT32_MAX
#elif defined(ZBUFFER16)
typedef uint16_t PingoDepthValue;
#define PINGO_DEPTH_MAX UINT16_MAX
#else
typedef uint8_t PingoDepthValue;
#define PINGO_DEPTH_MAX UINT8_MAX
#endif

// True when the pixel should be drawn, in which case the buffer is updated.
// The comparison is the one depth_check has always used: a stored value that
// is greater wins, and ZBUFFER8 stores the sense inverted.
static inline bool depth_test_and_write(PingoDepth *d, int idx, float value) {
  const PingoDepthValue v = (PingoDepthValue)(value * (float)PINGO_DEPTH_MAX);
#ifdef ZBUFFER8
  if (v > d[idx].d) {
    return false;
  }
#else
  if (v < d[idx].d) {
    return false;
  }
#endif
  d[idx].d = v;
  return true;
}

void depth_write(PingoDepth *d, int idx, float value);
bool depth_check(PingoDepth *d, int idx, float value);
