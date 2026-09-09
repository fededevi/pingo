
#pragma once

#include "example/common/expected.h"
#include "math/vec2.h"
#include "render/backend.h"

typedef struct {
  Backend backend;
  Vec2i size;
} LinuxWindowBackend;

PgError linux_window_backend_init(LinuxWindowBackend *this, Vec2i size);
