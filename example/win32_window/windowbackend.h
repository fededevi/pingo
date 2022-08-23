#pragma once

struct Renderer;

#include "render/backend.h"
#include "math/vec2.h"

typedef struct Pixel Pixel;
typedef  struct {
    BackEnd backend;
    Vec2i size;
} WindowBackEnd;

void windowBackEndInit(WindowBackEnd * thiss, Vec2i size);
