#pragma once

#include "frame.h"
#include "scene.h"

#include <stdint.h>

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct {
    Frame frameBuffer;
    Pixel clearColor;
    int clear;
    Scene * scene;
} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vector2I size, Pixel *fb0);

extern int rendererSetScene(Renderer *r, Scene *s);

#ifdef __cplusplus
    }
#endif
