#pragma once

#include "fwd.h"
#include "pixel.h"
#include "target.h"
#include "texture.h"
#include <stdbool.h>

/** The two matrices are only ever set and used together. */
typedef struct Camera {
  Mat4 projection;
  Mat4 view;
} Camera;

struct Renderer {
  Renderable *root_renderable;

  /** Colour and depth together; the backend supplies it. */
  RenderTarget target;

  Pixel clear_color;
  bool clear;

  Camera camera;

  Backend *backend;

  // Rendering optimizations. There is no early-Z flag: the depth test has
  // always run before the texture lookup, and the flag that claimed to
  // control it ran identical code in both branches.
  bool enable_backface_culling;
  bool enable_frustum_culling;
};

extern int renderer_render(Renderer *);

extern int renderer_init(Renderer *, Vec2i size, Backend *backend);

extern int renderer_set_root_renderable(Renderer *renderer, Renderable *root);

// Optimization configuration functions
extern void renderer_enable_backface_culling(Renderer *renderer, bool enable);
extern void renderer_enable_frustum_culling(Renderer *renderer, bool enable);
extern void renderer_set_all_optimizations(Renderer *renderer, bool enable);
