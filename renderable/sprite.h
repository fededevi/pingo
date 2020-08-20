#pragma once

#include "../render/frame.h"
#include "renderable.h"
#include "../math/mat3.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct  {
   Mat3 t;
   Frame frame;
} Sprite;

extern int spriteInit( Sprite * s, Frame f, Mat3 t);
extern int spriteRandomize( Sprite * s);
extern Renderable spriteAsRenderable( Sprite * s);

#ifdef __cplusplus
    }
#endif
