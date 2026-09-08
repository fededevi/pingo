#pragma once

#include "fwd.h"
#include "math/mat4.h"

/// A basic type which provide a render function pointer
typedef struct {
  int (*render)(void *this, Mat4 transform, Renderer *renderer);
} Renderable;
