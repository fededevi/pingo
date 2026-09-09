#pragma once

#include "fwd.h"
#include "renderable.h"

typedef struct Object {
  Renderable renderable;
  Mesh *mesh;
  Material *material;
} Object;

PINGO_ASSERT_RENDERABLE_FIRST(Object);

extern int object_init(Object *this, Mesh *mesh, Material *material);
