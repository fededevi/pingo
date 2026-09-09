#pragma once

#include "fwd.h"
#include "math/vec4.h"

/**
 * Provides a common interface to multiple graphical Backends
 */

struct Backend {
  // Called on initialization and re-initialization
  void (*init)(Renderer *, struct Backend *, Vec4i rect);

  // Called before starting rendering
  void (*before_render)(Renderer *, struct Backend *);

  // Called after having finished a render
  void (*after_render)(Renderer *, struct Backend *);

  // Should return the surface to draw into: the colour buffer, its size, and
  // a depth buffer with one entry per pixel of it. Replaces the pair of
  // getters this used to have, which returned bare pointers and left the
  // depth buffer's size implicit - the comment here used to describe it in
  // units of Pixel, which it never was.
  RenderTarget *(*get_target)(Renderer *, struct Backend *);
};
