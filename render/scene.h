#pragma once

#include <stdint.h>
#include "frame.h"
#include "renderable.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define MAX_SCENE_RENDERABLES 32

typedef struct {
    uint8_t numberOfRenderables;
    Renderable renderables[MAX_SCENE_RENDERABLES];
} Scene;

extern int sceneInit(Scene * s);
extern int sceneAddRenderable(Scene * scene, const Renderable renderable);

#ifdef __cplusplus
    }
#endif
