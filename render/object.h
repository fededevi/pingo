#pragma once

#include "renderable.h"

typedef struct Mesh Mesh;
typedef struct Material Material;

typedef struct Object {
  Renderable renderable;
  Mesh *mesh;
  Material *material;
} Object;

extern int object_init(Object *this, Mesh *mesh, Material *material);
