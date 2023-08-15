#pragma once

#include "math/vec4.h"
#include "pixel.h"
#include "texture.h"
#include <stdbool.h>

typedef struct BackEnd BackEnd;

typedef struct Renderer {
  Vec4i camera;
  Renderable *root_renderable;

  Texture frameBuffer;
  Pixel clearColor;
  bool clear;

  Mat4 camera_projection;
  Mat4 camera_view;

  BackEnd *backEnd;

} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, struct BackEnd *backEnd);

extern int renderer_set_root_renderable(Renderer *renderer, Renderable *root);

extern int rendererSetCamera(Renderer *r, Vec4i camera);
