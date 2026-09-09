#pragma once

#include "fwd.h"
#include "math/vec2.h"
#include "pixel.h"

#include <stdint.h>
#include "renderable.h"

struct Texture {
  Vec2i size;
  Pixel *pixels;
};

extern int texture_init(Texture *f, Vec2i size, Pixel *);

extern int texture_init_rgba_file(Texture *f, Vec2i size, char *filename);

extern Renderable texture_as_renderable(Texture *s);

extern void texture_draw(Texture *f, Vec2i pos, Pixel color);

// The rasterizer already holds the linear index, having needed it for the
// depth buffer. Going back through x and y makes texture_draw recompute
// x + y * width, a multiply per pixel drawn, for a result already known.
static inline void texture_draw_index(Texture *f, int index, Pixel color) {
  f->pixels[index] = color;
}

extern Pixel texture_read(Texture *f, Vec2i pos);

/**
 * Samples a power-of-two texture, given size-1 as masks.
 *
 * The caller decides once that the texture is power-of-two and computes the
 * masks; doing it here meant two ands and two compares per textured pixel to
 * re-establish something fixed for the whole object, and left the general
 * path's code sitting in the innermost loop.
 */
static inline Pixel texture_read_uv_pow2(const Texture *f, Vec2f pos,
                                         int w_mask, int h_mask) {
  // Two's complement makes the mask wrap negatives without a correction step.
  const int sx = (int)(pos.x * (w_mask + 1)) & w_mask;
  const int sy = (int)(pos.y * (h_mask + 1)) & h_mask;
  return f->pixels[sx + sy * (w_mask + 1)];
}

/** True when both dimensions are powers of two, so masking can be used. */
static inline int texture_is_pow2(const Texture *f) {
  const int w = f->size.x;
  const int h = f->size.y;
  return (w & (w - 1)) == 0 && (h & (h - 1)) == 0;
}

// Inline for the same reason as pixel_mul: one call per textured pixel.
static inline Pixel texture_read_uv(Texture *f, Vec2f pos) {
  const int w = f->size.x;
  const int h = f->size.y;

  // Converting a negative float to an unsigned type is undefined, and the
  // interpolated coordinate can go negative just outside a triangle, so the
  // wrap happens in signed arithmetic before the cast.
  int sx = (int)(pos.x * w);
  int sy = (int)(pos.y * h);

  // For a power-of-two texture the wrap is a mask rather than an integer
  // division, and two's complement makes it land non-negative unaided. Every
  // texture in the project is power-of-two; the general path is for the rest.
  if ((w & (w - 1)) == 0 && (h & (h - 1)) == 0) {
    return f->pixels[(sx & (w - 1)) + (sy & (h - 1)) * w];
  }

  sx %= w;
  sy %= h;
  const uint16_t x = (uint16_t)(sx < 0 ? sx + w : sx);
  const uint16_t y = (uint16_t)(sy < 0 ? sy + h : sy);
  return f->pixels[x + y * (uint32_t)w];
}
