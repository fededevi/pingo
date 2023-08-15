#pragma once

#include "math/vec2.h"
#include "render/backend.h"

typedef struct TerminalBackend {
  Backend backend;
} TerminalBackend;

void terminal_backend_init(TerminalBackend *t, Vec2i size);
