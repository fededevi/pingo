#pragma once

#include "pixel.h"
#include "texture.h"
#include <stdbool.h>

typedef struct Backend Backend;

typedef struct Renderer {
  Renderable *root_renderable;

  Texture framebuffer;
  Pixel clear_color;
  bool clear;

  Mat4 camera_projection;
  Mat4 camera_view;

  Backend *backend;

  // Rendering optimizations
  bool enable_backface_culling;
  bool enable_frustum_culling;
  bool enable_early_z_test;

} Renderer;

extern int renderer_render(Renderer *);

extern int renderer_init(Renderer *, Vec2i size, Backend *backend);

extern int renderer_set_root_renderable(Renderer *renderer, Renderable *root);

// Optimization configuration functions
extern void renderer_enable_backface_culling(Renderer *renderer, bool enable);
extern void renderer_enable_frustum_culling(Renderer *renderer, bool enable);
extern void renderer_enable_early_z_test(Renderer *renderer, bool enable);
extern void renderer_set_all_optimizations(Renderer *renderer, bool enable);
