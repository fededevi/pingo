#pragma once

#include "renderable.h"
#include "texture.h"

typedef struct Sprite {
  Renderable renderable;
  Texture texture;
} Sprite;

PINGO_ASSERT_RENDERABLE_FIRST(Sprite);

extern int sprite_init(Sprite *this, Texture texture);
extern int sprite_randomize(Sprite *this);
