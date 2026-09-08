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
  void (*beforeRender)(Renderer *, struct Backend *);

  // Called after having finished a render
  void (*afterRender)(Renderer *, struct Backend *);

  // Should return the address of the buffer (height*width*sizeof(Pixel))
  Pixel *(*getFrameBuffer)(Renderer *, struct Backend *);

  // Should return the address of the buffer (height*width*sizeof(Pixel))
  PingoDepth *(*getZetaBuffer)(Renderer *, struct Backend *);
};
