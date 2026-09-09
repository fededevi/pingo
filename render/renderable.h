#pragma once

#include "fwd.h"
#include "math/mat4.h"

#include <stddef.h>

/// A basic type which provide a render function pointer
typedef struct {
  int (*render)(void *this, Mat4 transform, Renderer *renderer);
} Renderable;

/**
 * Every drawable embeds a Renderable as its first member, and callers cast
 * its address straight to Renderable *. That only works while the member sits
 * at offset zero, and reordering the fields would still compile and then
 * dispatch through the wrong bytes - so each type asserts it.
 *
 * C99 has no _Static_assert, hence the negative array size.
 */
#define PINGO_ASSERT_RENDERABLE_FIRST(type)                                    \
  typedef char pingo_##type##_renderable_must_be_first                         \
      [(offsetof(type, renderable) == 0) ? 1 : -1]
