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

} Renderer;

extern int renderer_render(Renderer *);

extern int renderer_init(Renderer *, Vec2i size, Backend *backend);

extern int renderer_set_root_renderable(Renderer *renderer, Renderable *root);
