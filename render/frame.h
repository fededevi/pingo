#pragma once

#include "pixel.h"
#include "renderable.h"
#include "../math/vec2.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct  Frame {
   Vec2i size;
   Pixel * frameBuffer;
} Frame;

extern int frameInit( Frame * f, Vec2i size, Pixel *);

extern Renderable frameAsRenderable( Frame * s);

extern void  frameDraw(Frame * f, Vec2i pos, Pixel color);

extern Pixel frameRead(Frame * f, Vec2i pos);

extern Pixel frameReadBilinear(Frame *f, Vec2f pos);

#ifdef __cplusplus
    }
#endif
