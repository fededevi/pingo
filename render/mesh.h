#pragma once

#include <stdint.h>

#include "fwd.h"
#include "math/vec2.h"
#include "math/vec3.h"

struct Mesh {
  int indexes_count;
  // const so the tables can live in .rodata: on a microcontroller that means
  // flash rather than RAM copied at startup, which for the shipped meshes is
  // the difference between 174 KB of RAM and none.
  const uint16_t *pos_indices;
  const uint16_t *tex_indices;
  const Vec3f *positions;
  const Vec2f *textCoord;
};
