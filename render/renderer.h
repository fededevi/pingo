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
    Frame frameBuffer;
    Pixel clearColor;
    int clear;
    Scene * scene;
    BackEnd * backEnd;
} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vector2I size, struct BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

static int (*renderingFunctions[RENDERABLE_COUNT])(Renderer *, Renderable);

#ifdef __cplusplus
    }
#endif
