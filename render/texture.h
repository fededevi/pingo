#pragma once

#include "fwd.h"
#include "math/vec2.h"
#include "pixel.h"
#include "renderable.h"

struct Texture {
  Vec2i size;
  Pixel *frameBuffer;
};

extern int texture_init(Texture *f, Vec2i size, Pixel *);

extern int texture_init_rgbafile(Texture *f, Vec2i size, char *filename);

extern Renderable texture_as_renderable(Texture *s);

extern void texture_draw(Texture *f, Vec2i pos, Pixel color);

// The rasterizer already holds the linear index, having needed it for the
// depth buffer. Going back through x and y makes texture_draw recompute
// x + y * width, a multiply per pixel drawn, for a result already known.
static inline void texture_draw_index(Texture *f, int index, Pixel color) {
  f->frameBuffer[index] = color;
}

extern Pixel texture_read(Texture *f, Vec2i pos);

extern Pixel texture_readF(Texture *f, Vec2f pos);
