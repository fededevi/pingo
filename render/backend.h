#pragma once

#include "math/vec4.h"

/**
 * Provides a common interface to multiple graphical Backends
 */

typedef struct Renderer Renderer;
typedef struct Pixel Pixel;
typedef struct PingoDepth PingoDepth;
typedef struct Texture Texture;

typedef struct Backend {
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
} Backend;
