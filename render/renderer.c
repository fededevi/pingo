#include "renderer.h"
#include "backend.h"
#include "depth.h"
#include "math/mat4.h"
#include "pixel.h"
#include "render/state.h"
#include <stdio.h>
#include <string.h>

int renderer_init(Renderer *r, Vec2i size, Backend *backend) {
  IF_NULL_RETURN(r, INIT_ERROR);
  IF_NULL_RETURN(backend, INIT_ERROR);

  r->root_renderable = 0;
  r->clear = 1;
  r->clear_color = PIXELBLACK;
  r->backend = backend;
  r->backend->init(r, r->backend, (Vec4i){0, 0, 0, 0});

  r->enable_backface_culling = true;
  r->enable_frustum_culling = false; // Disabled by default (can have overhead)

  const RenderTarget *t = backend->getTarget(r, backend);
  IF_NULL_RETURN(t, INIT_ERROR);

  // The size the caller asked for wins; the backend's buffers have to match it,
  // and saying so here is what the two separate getters could not.
  return render_target_init(&r->target, size, t->color.frameBuffer, t->depth);
}

int renderer_render(Renderer *r) {
  IF_NULL_RETURN(r, RENDER_ERROR);
  IF_NULL_RETURN(r->root_renderable, RENDER_ERROR);

  Backend *be = r->backend;

  be->beforeRender(r, be);

  // Re-read every frame: a backend may hand back a different buffer each time,
  // as a double-buffered one does. One call now, where this used to make four.
  const RenderTarget *t = be->getTarget(r, be);
  IF_NULL_RETURN(t, RENDER_ERROR);
  r->target.color.frameBuffer = t->color.frameBuffer;
  r->target.depth = t->depth;

  const int pixels = r->target.color.size.x * r->target.color.size.y;

  if (r->target.depth != 0) {
    memset(r->target.depth, 0, (size_t)pixels * sizeof(PingoDepth));
  }

  if (r->clear) {
    memset(r->target.color.frameBuffer, 0, (size_t)pixels * sizeof(Pixel));
  }

  r->root_renderable->render(r->root_renderable, mat4Identity(), r);

  be->afterRender(r, be);

  return OK;
}

int renderer_set_root_renderable(Renderer *renderer, Renderable *root) {
  IF_NULL_RETURN(renderer, SET_ERROR);
  IF_NULL_RETURN(root, SET_ERROR);

  renderer->root_renderable = root;
  return OK;
}

// Optimization configuration functions
void renderer_enable_backface_culling(Renderer *renderer, bool enable) {
  if (renderer) {
    renderer->enable_backface_culling = enable;
  }
}

void renderer_enable_frustum_culling(Renderer *renderer, bool enable) {
  if (renderer) {
    renderer->enable_frustum_culling = enable;
  }
}

void renderer_set_all_optimizations(Renderer *renderer, bool enable) {
  if (renderer) {
    renderer->enable_backface_culling = enable;
    renderer->enable_frustum_culling = enable;
  }
}
