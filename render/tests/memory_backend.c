#include "memory_backend.h"

#include "render/renderer.h"
#include "render/state.h"

#include <stdlib.h>

// The vtable takes no user pointer, so the buffers are reached through the
// backend pointer each call rather than through globals.
static void mb_init(Renderer *renderer, Backend *backend, Vec4i rect) {
  (void)renderer;
  (void)backend;
  (void)rect;
}

static void mb_before_render(Renderer *renderer, Backend *backend) {
  (void)renderer;
  (void)backend;
}

static void mb_after_render(Renderer *renderer, Backend *backend) {
  (void)renderer;
  (void)backend;
}

static RenderTarget *mb_get_target(Renderer *renderer, Backend *backend) {
  (void)renderer;
  return &((MemoryBackend *)backend)->target;
}

int memory_backend_init(MemoryBackend *this, Vec2i size) {
  if (this == NULL || size.x <= 0 || size.y <= 0) {
    return 1;
  }

  this->size = size;
  this->backend.init = &mb_init;
  this->backend.before_render = &mb_before_render;
  this->backend.after_render = &mb_after_render;
  this->backend.get_target = &mb_get_target;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  this->frame = calloc(pixels, sizeof(Pixel));
  this->depth = calloc(pixels, sizeof(PingoDepth));
  if (this->frame == NULL || this->depth == NULL) {
    memory_backend_free(this);
    return 1;
  }

  if (render_target_init(&this->target, size, this->frame, this->depth) != OK) {
    memory_backend_free(this);
    return 1;
  }

  return 0;
}

void memory_backend_free(MemoryBackend *this) {
  if (this == NULL) {
    return;
  }
  free(this->frame);
  this->frame = NULL;
  free(this->depth);
  this->depth = NULL;
}
