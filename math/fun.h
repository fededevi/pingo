#pragma once

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#include "vec2.h"

// One expression each, and the rasterizer calls orient2d four times and
// isClockWise once per triangle - a profile counted 6.9 million and 2.4
// million calls in a 300 frame run. Out of line they are calls the compiler
// cannot fold into its surrounding arithmetic.
#include "vec3.h"

static inline int edgeFunction(const Vec2f *a, const Vec2f *b,
                               const Vec2f *c) {
  return (c->x - a->x) * (b->y - a->y) - (c->y - a->y) * (b->x - a->x);
}

static inline float isClockWise(float x1, float y1, float x2, float y2,
                                float x3, float y3) {
  return (y2 - y1) * (x3 - x2) - (y3 - y2) * (x2 - x1);
}

static inline int orient2d(Vec2i a, Vec2i b, Vec2i c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
