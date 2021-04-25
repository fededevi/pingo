#pragma once

#include "../render/backend.h"
#include "../math/vec2.h"

typedef struct Pixel Pixel;
typedef struct Depth Depth;
typedef struct Texture Texture;

typedef  struct {
    BackEnd backend;
    Depth * zetaBuffer;
    Vec2i size;
} TTGOBackend;

void ttgoBackendInit(TTGOBackend * ths, Vec2i size);

void texture_draw(Texture *f, Vec2i pos, Pixel color);
