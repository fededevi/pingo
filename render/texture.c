#include "texture.h"
#include "render/state.h"
#include <stdio.h>

int texture_init(Texture *f, Vec2i size, Pixel *buf) {
  if (size.x * size.y == 0)
    return 1; // 0 sized rect

  if (buf == 0)
    return 2; // null ptr buffer

  f->frameBuffer = (Pixel *)buf;
  f->size = size;

  return OK;
}

void texture_draw(Texture *f, Vec2i pos, Pixel color) {
  f->frameBuffer[pos.x + pos.y * f->size.x] = color;
}

Pixel texture_read(Texture *f, Vec2i pos) {
  return f->frameBuffer[pos.x + pos.y * f->size.x];
}

Pixel texture_readF(Texture *f, Vec2f pos) {
  const int w = f->size.x;
  const int h = f->size.y;

  // Converting a negative float to an unsigned type is undefined, and the
  // interpolated coordinate can go negative just outside a triangle, so the
  // wrap happens in signed arithmetic before the cast.
  int sx = (int)(pos.x * w);
  int sy = (int)(pos.y * h);

  // The modulo is an integer division, once per axis for every textured pixel
  // drawn - the single most expensive operation in the inner loop, and on the
  // ARM and MIPS targets there is no divide instruction to do it with. For a
  // power-of-two texture the same wrap is a mask, and two's complement makes
  // it come out non-negative without the correction below.
  if ((w & (w - 1)) == 0 && (h & (h - 1)) == 0) {
    return f->frameBuffer[(sx & (w - 1)) + (sy & (h - 1)) * w];
  }

  sx %= w;
  sy %= h;
  const uint16_t x = (uint16_t)(sx < 0 ? sx + w : sx);
  const uint16_t y = (uint16_t)(sy < 0 ? sy + h : sy);
  return f->frameBuffer[x + y * (uint32_t)w];
}
