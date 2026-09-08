#pragma once

#include "example/common/expected.h"
#include "math/vec2.h"
#include "render/backend.h"

typedef struct TerminalBackend {
  Backend backend;
} TerminalBackend;

PgError terminal_backend_init(TerminalBackend *t, Vec2i size);
