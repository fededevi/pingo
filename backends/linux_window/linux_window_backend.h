
#pragma once

#include "math/vec2.h"
#include "render/backend.h"

typedef struct Pixel Pixel;

typedef struct {
  Backend backend;
  Vec2i size;
} LinuxWindowBackend;

void linuxWindowBackendInit(LinuxWindowBackend *thiss, Vec2i size);
