#pragma once

#include "../render/backend.h"
#include "../math/vec2.h"

typedef  struct LinuxTermBackend {
    BackEnd backend;
} LinuxTermBackend;

void linuxterm_backend_init(LinuxTermBackend * t, Vec2i size);
