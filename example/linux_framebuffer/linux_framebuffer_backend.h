#pragma once

#include "example/common/expected.h"
#include "math/vec2.h"
#include "render/backend.h"

typedef struct {
  Backend backend;
  Vec2i size;
} LinuxFramebufferBackend;

/**
 * Initializes the backend for a size.x by size.y surface on /dev/fb0.
 * On failure the caller must still hand the backend to destroy_backend, which
 * releases whatever was acquired before the failure.
 */
PgError linux_framebuffer_backend_init(LinuxFramebufferBackend *this,
                                       Vec2i size);
