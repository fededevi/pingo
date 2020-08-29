#pragma once

#include "frame.h"
#include "renderable.h"
#include "pixel.h"
#include "../math/vec4.h"

/**
  * Defines the type of filtering used when textures are resized and rotated
  * by a transformation. Nearest filtering just take 1 texture sample in the
  * single pixel which the source coordinate is tranformed to.
  * Bilinear filtering takes 4 samples on the source texture to compute a
  * weighted average of those 4 values based on the distance of the input
  * point.
  * Anisotropic filtering takes 4/16 samples from the source image
  * distributed uniformly over the area occupied by the destination pixel on
  * the source image and makes an average of those values.
  */
//#define FILTERING_NEAREST
//#define FILTERING_BILINEAR
//#define FILTERING_ANISOTROPIC
#define FILTERING_ANISOTROPICX2

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct Scene Scene;
typedef struct BackEnd BackEnd;

typedef struct Renderer{
    Vec4i camera;
    Frame frameBuffer;
    Pixel clearColor;
    int clear;
    Scene * scene;
    BackEnd * backEnd;
} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, struct BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

extern int rendererSetCamera(Renderer *r, Vec4i camera);

#ifdef __cplusplus
    }
#endif
