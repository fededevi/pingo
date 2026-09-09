#pragma once

#include "renderable.h"
#include "texture.h"

typedef struct Sprite {
  Renderable renderable;
  Texture texture;
} Sprite;

PINGO_ASSERT_RENDERABLE_FIRST(Sprite);

/** The vtable entry, declared so it can be called and tested
 * directly and not only through Renderable.render. */
extern int sprite_render(void *this, Mat4 parent, Renderer *renderer);

extern int sprite_init(Sprite *this, Texture texture);
extern int sprite_randomize(Sprite *this);
