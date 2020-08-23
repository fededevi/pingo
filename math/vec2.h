#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vec2i {
    int x;
    int y;
} Vec2i;

typedef struct {
    float x;
    float y;
} Vec2f;

#define VECTORZERO (Vector2I){0,0}

extern Vec2i vector2ISum(Vec2i l, Vec2i r);

extern Vec2f vecItoF(Vec2i v);

extern Vec2i vecFtoI(Vec2f v);

#ifdef __cplusplus
}
#endif
