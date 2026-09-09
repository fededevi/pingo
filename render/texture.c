#include "texture.h"
#include "render/state.h"
#include <stdio.h>

int texture_init(Texture *f, Vec2i size, Pixel *buf) {
  if (size.x * size.y == 0)
    return 1; // 0 sized rect

  if (buf == 0)
    return 2; // null ptr buffer

  f->pixels = (Pixel *)buf;
  f->size = size;

  return OK;
}

void texture_draw(Texture *f, Vec2i pos, Pixel color) {
  f->pixels[pos.x + pos.y * f->size.x] = color;
}

Pixel texture_read(Texture *f, Vec2i pos) {
  return f->pixels[pos.x + pos.y * f->size.x];
}

