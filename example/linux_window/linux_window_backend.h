
#pragma once

#include "render/backend.h"
#include "math/vec2.h"

typedef struct Pixel Pixel;

typedef  struct {
    BackEnd backend;
    Vec2i size;
} LinuxWindowBackEnd;


void linuxWindowBackEndInit(LinuxWindowBackEnd * thiss, Vec2i size);
