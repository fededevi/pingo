#pragma once

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#include "vec2.h"

// Function declarations
int edgeFunction(const Vec2f *a, const Vec2f *b, const Vec2f *c);
float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3);
int orient2d(Vec2i a, Vec2i b, Vec2i c);
