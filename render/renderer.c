#include "renderer.h"
#include "renderable/sprite.h"
#include "renderable/qrcode.h"

#include <string.h>
#include <stdio.h>


int drawRect(Vector2I off, Renderer *r, Frame * src) {
    Frame des = rendererDrawBuffer(r);

    for (int y = 0; y < src->size.y; y++ ) {
        if (y > des.size.y)
            break; //Do not draw outside bounds

        for (int x = 0; x < src->size.x; x++ ) {
            if (x > des.size.x)
                break; //Do not draw outside bounds

            Vector2I srcPos = {x,y};
            Vector2I desPos = vector2ISum(off,srcPos);
            Pixel color = frameRead(src, srcPos);
            frameDraw(&des, desPos, color);
        }
    }
    return 0;
}

int drawRectTransform(Transform t, Renderer *r, Frame * src) {
    Frame des = rendererDrawBuffer(r);

    for (int y = 0; y < src->size.y; y++ ) {
        if (y > des.size.y)
            break; //Do not draw outside bounds

        for (int x = 0; x < src->size.x; x++ ) {
            if (x > des.size.x)
                break; //Do not draw outside bounds

            //Vector2I off = transformMultiply(,&t);
            Vector2I srcPos = {x,y};
            Vector2I desPos = transformMultiply(&srcPos,&t);
            Pixel color = frameRead(src, srcPos);
            frameDraw(&des, desPos, color);
        }
    }
    return 0;
}

int renderFrame(Renderer *r, Renderable ren) {
    Frame * f = ren.impl;
    return drawRect((Vector2I){0,0},r,f);
};

int renderSprite(Renderer *r, Renderable ren) {
    Sprite * s = (ren.impl);
    return drawRectTransform(s->t,r,&s->frame);
};

int renderQrCode(Renderer *r, Renderable ren) {
    QrCode * qr = (ren.impl);
    return drawRectTransform(qr->sprite.t,r,&qr->sprite.frame);
};

int (*renderingFunctions[RENDERABLE_COUNT])(Renderer *, Renderable)={&renderFrame, &renderSprite, &renderQrCode};

int rendererInit(Renderer * r, Vector2I size, Pixel *fb0, Pixel *fb1) {
    r->scene = 0;

    r->currentBuffer = 0;

    int e = 0;

    e = frameInit(&(r->frameBuffers[0]), size, fb0);
    if (e) return e;

    e = frameInit(&(r->frameBuffers[1]), size, fb1);
    if (e) return e;

    return 0; //No error
}

Frame rendererCurrentBuffer(Renderer * r) {
    return r->frameBuffers[r->currentBuffer];
}

Frame rendererDrawBuffer(Renderer * r) {
    return r->frameBuffers[!r->currentBuffer];
}

void rendererSwap(Renderer * r) {
    Pixel * p0 = r->frameBuffers[0].frameBuffer;
    Pixel * p1 = r->frameBuffers[1].frameBuffer;
    r->frameBuffers[0].frameBuffer = p1;
    r->frameBuffers[1].frameBuffer = p0;
}

int rendererRender(Renderer * renderer)
{
    //Clear draw buffer before rendering
    Frame des = rendererDrawBuffer(renderer);
    memset(des.frameBuffer,255,des.size.x*des.size.y*sizeof (Pixel));

    for (int i = 0; i < renderer->scene->numberOfRenderables; i++) {
        Renderable r = renderer->scene->renderables[i];
        renderingFunctions[r.renderableType](renderer,r);
    }

    return 0;
}

int rendererSetScene(Renderer *r, Scene *s)
{
    if (s == 0)
        return 1; //nullptr scene

    r->scene = s;
    return 0;
}
