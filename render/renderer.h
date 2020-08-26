#pragma once

#include "frame.h"
#include "renderable.h"
#include "pixel.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct Scene Scene;
typedef struct BackEnd BackEnd;

typedef struct Renderer{
    Vec2i offset;
    Frame frameBuffer;
    Pixel clearColor;
    int clear;
    Scene * scene;
    BackEnd * backEnd;
} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, struct BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

extern int rendererSetArea(Renderer *r, Vec2i offset, Vec2i size);

#ifdef __cplusplus
    }
#endif
