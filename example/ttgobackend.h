#pragma once

#include "../render/backend.h"
#include "../math/vec2.h"

typedef struct Pixel Pixel;
typedef struct Depth Depth;

typedef  struct {
    BackEnd backend;
    Depth * zetaBuffer;
    Pixel * frameBuffer;
    Vec2i size;
} TTGOBackend;

void ttgoBackendInit(TTGOBackend * ths, Pixel * buf, Vec2i size);

