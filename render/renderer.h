#pragma once

#include "frame.h"
#include "scene.h"

#include <stdint.h>

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct {
    uint8_t currentBuffer;
    Frame frameBuffers[2];
    Scene * scene;
} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vector2I size, Pixel *fb0, Pixel *fb1);

extern Frame rendererCurrentBuffer(Renderer *r);

extern Frame rendererDrawBuffer(Renderer *r);

extern int rendererSetScene(Renderer *r, Scene *s);

extern void rendererSwap(Renderer *);

#ifdef __cplusplus
    }
#endif
