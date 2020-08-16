#pragma once

#include "render/frame.h"
#include "renderable.h"
#include "../math/transform.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct  {
   Transform t;
   Frame frame;
} Sprite;

extern int spriteInit( Sprite * s, Frame f, Transform t);
extern int spriteRandomize( Sprite * s);
extern Renderable spriteAsRenderable( Sprite * s);

#ifdef __cplusplus
    }
#endif
