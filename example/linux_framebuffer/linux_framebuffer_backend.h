
#pragma once

#include "render/backend.h"
#include "math/vec2.h"

typedef struct Pixel Pixel;

typedef  struct {
    BackEnd backend;
    Vec2i size;
} LinuxFramebufferBackEnd;


void linuxFramebufferBackEndInit(LinuxFramebufferBackEnd * this, Vec2i size);

