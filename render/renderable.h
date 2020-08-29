#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  RenderableType {
    RENDERABLE_SPRITE=0,
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

