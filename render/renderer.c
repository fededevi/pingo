#include <string.h>
#include <stdio.h>

#include "renderer.h"
#include "sprite.h"
#include "pixel.h"
#include "backend.h"
#include "scene.h"
#include "rasterizer.h"

static int (*renderingFunctions[RENDERABLE_COUNT])(Mat3 transform, Renderer *, Renderable);

int renderFrame(Renderer *r, Renderable ren) {
    Frame * f = ren.impl;
    return drawRect((Vec2i){0,0},r,f);
};

int renderSprite(Mat3 transform, Renderer *r, Renderable ren) {
    Sprite * s = ren.impl;
    Mat3 backUp = s->t;

    //Apply parent transform to the local transform
    s->t = mat3MultiplyM( &s->t, &transform);

    //Apply camera translation
    Mat3 newMat = mat3Translate((Vec2f){-r->camera.x,-r->camera.y});
    s->t = mat3MultiplyM(&s->t, &newMat);

    if (mat3IsOnlyTranslation(&s->t)) {
        Vec2i off = {s->t.elements[2], s->t.elements[5]};
        drawRect(off,r, &s->frame);
        s->t = backUp;
        return 0;
    }

    if (mat3IsOnlyTranslationDoubled(&s->t)) {
        Vec2i off = {s->t.elements[2], s->t.elements[5]};
        drawRectDoubled(off,r, &s->frame);
        s->t = backUp;
        return 0;
    }

    drawRectTransform(s->t,r,&s->frame);
    s->t = backUp;
    return 0;
};



void renderRenderable(Mat3 transform, Renderer *r, Renderable ren) {
    renderingFunctions[ren.renderableType](transform, r, ren);
};

int renderScene(Mat3 transform, Renderer *r, Renderable ren) {
    Scene * s = ren.impl;
    if (!s->visible)
        return 0;

    //Apply hierarchy transfom
    Mat3 newTransform = mat3MultiplyM(&s->transform,&transform);
    for (int i = 0; i < s->numberOfRenderables; i++) {
        renderRenderable(newTransform, r, s->renderables[i]);
    }
    return 0;
};

int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd) {
    renderingFunctions[RENDERABLE_SPRITE] = &renderSprite;
    renderingFunctions[RENDERABLE_SCENE] = &renderScene;

    r->scene = 0;
    r->clear = 1;
    r->clearColor = PIXELBLACK;
    r->backEnd = backEnd;

    r->backEnd->init(r, r->backEnd, (Vec4i){0,0,0,0});

    int e = 0;
    e = frameInit(&(r->frameBuffer), size, backEnd->getFrameBuffer(r, backEnd));
    if (e) return e;


    return 0;
}

/**
 * @brief Clear the whole framebuffer slowly. This prevents underrun in the FPGA draw buffer and prevents desynchronization
 * of the video signal
 */
void clearBufferSlowly (Frame f)
{
    int length = f.size.x*sizeof (Pixel);
    for (int y = 0; y < f.size.y; y++) {
        uint16_t offset = f.size.x*sizeof (Pixel);
        memset(f.frameBuffer+offset*y,0,length);
    }
}


int rendererRender(Renderer * r)
{
    r->backEnd->beforeRender(r, r->backEnd);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer= r->backEnd->getFrameBuffer(r, r->backEnd);

    //Clear draw buffer before rendering
    if (r->clear) {
        clearBufferSlowly(r->frameBuffer);
    }

    renderScene(mat3Identity(), r, sceneAsRenderable(r->scene));

    r->backEnd->afterRender(r, r->backEnd);

    return 0;
}

int rendererSetScene(Renderer *r, Scene *s)
{
    if (s == 0)
        return 1; //nullptr scene

    r->scene = s;
    return 0;
}

int rendererSetCamera(Renderer *r, Vec4i rect)
{
    r->camera = rect;
    r->backEnd->init(r, r->backEnd, rect);
    r->frameBuffer.size = (Vec2i){rect.z, rect.w};
    return 0;
}
