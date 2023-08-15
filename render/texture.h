#pragma once

#include "math/vec2.h"
#include "pixel.h"
#include "renderable.h"

typedef struct Texture {
  Vec2i size;
  Pixel *frameBuffer;
} Texture;

extern int texture_init(Texture *f, Vec2i size, Pixel *);

extern int texture_init_rgbafile(Texture *f, Vec2i size, char *filename);

extern Renderable texture_as_renderable(Texture *s);

extern void texture_draw(Texture *f, Vec2i pos, Pixel color);

extern Pixel texture_read(Texture *f, Vec2i pos);

extern Pixel texture_readF(Texture *f, Vec2f pos);
