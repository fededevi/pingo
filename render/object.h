#pragma once

#include "fwd.h"
#include "renderable.h"

typedef struct Object {
  Renderable renderable;
  Mesh *mesh;
  Material *material;
} Object;

PINGO_ASSERT_RENDERABLE_FIRST(Object);

/** The vtable entry, declared so it can be called and tested
 * directly and not only through Renderable.render. */
extern int object_render(void *this, Mat4 parent, Renderer *renderer);

extern int object_init(Object *this, Mesh *mesh, Material *material);
