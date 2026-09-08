#pragma once

#include "fwd.h"
#include "texture.h"

struct Material {
  Texture *texture;
};

int material_init(Material *this, Texture *texture);
