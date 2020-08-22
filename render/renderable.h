#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  RenderableType {
    RENDERABLE_FRAME  =0,
    RENDERABLE_SPRITE,
    RENDERABLE_SCENE,
    RENDERABLE_COUNT,
} RenderableType;

typedef struct Renderable {
    RenderableType renderableType;
    void * impl;
} Renderable;


#ifdef __cplusplus
}
#endif

