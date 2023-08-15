#pragma once

#include "texture.h"

typedef struct Material {
  Texture *texture;
} Material;

int material_init(Material *this, Texture *texture);
