#pragma once

#include "texture.h"
#include "renderable.h"
#include "../math/mat3.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct Sprite {
   Mat3 t;
   Texture frame;
} Sprite;

extern int spriteInit( Sprite * s, Texture f, Mat3 t);
extern int spriteRandomize( Sprite * s);
extern Renderable spriteAsRenderable( Sprite * s);

#ifdef __cplusplus
    }
#endif
