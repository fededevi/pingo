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
  // Converting a negative float to an unsigned type is undefined, and the
  // interpolated coordinate can go negative just outside a triangle, so the
  // wrap happens in signed arithmetic before the cast.
  int sx = (int)(pos.x * f->size.x) % f->size.x;
  int sy = (int)(pos.y * f->size.y) % f->size.y;
  uint16_t x = (uint16_t)(sx < 0 ? sx + f->size.x : sx);
  uint16_t y = (uint16_t)(sy < 0 ? sy + f->size.y : sy);
  uint32_t index = x + y * f->size.x;
  Pixel value = f->frameBuffer[index];
  return value;
}
