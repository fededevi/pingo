#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t x;
    uint16_t y;
} Vector2I;

typedef struct {
    float x;
    float y;
} Vec2f;

#define VECTORZERO (Vector2I){0,0}

extern Vector2I vector2ISum(Vector2I l, Vector2I r);

#ifdef __cplusplus
}
#endif
