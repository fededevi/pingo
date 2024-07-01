#pragma once

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#include "vec2.h"

int edgeFunction(const Vec2f *a, const Vec2f *b, const Vec2f *c) {
  return (c->x - a->x) * (b->y - a->y) - (c->y - a->y) * (b->x - a->x);
}

float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3) {
  return (y2 - y1) * (x3 - x2) - (y3 - y2) * (x2 - x1);
}

int orient2d(Vec2i a, Vec2i b, Vec2i c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
