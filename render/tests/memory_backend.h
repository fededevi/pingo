#pragma once

#include "math/vec2.h"
#include "render/backend.h"
#include "render/depth.h"
#include "render/pixel.h"

/**
 * A Backend that renders into plain memory.
 *
 * It touches no OS facility at all, which is what makes it usable as a test
 * fixture on every platform - including the cross-compiled ones, where the
 * window and framebuffer backends do not exist.
 */
typedef struct MemoryBackend {
  Backend backend;
  Vec2i size;
  Pixel *frame;
  PingoDepth *depth;
} MemoryBackend;

/** Allocates the buffers. Returns 0 on success. */
int memory_backend_init(MemoryBackend *this, Vec2i size);

void memory_backend_free(MemoryBackend *this);
