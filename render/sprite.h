#pragma once

#include "frame.h"
#include "renderable.h"
#include "../math/rectI.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct  {
   Vector2I position;
   Frame frame;
} Sprite;

extern int spriteInit( Sprite * s, Frame f, Vector2I position);
extern int spriteRandomize( Sprite * s);
extern Renderable spriteAsRenderable( Sprite * s);

#ifdef __cplusplus
    }
#endif
