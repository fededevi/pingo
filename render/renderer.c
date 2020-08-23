#include <string.h>
#include <stdio.h>
#include <math.h>

#include "renderer.h"
#include "sprite.h"
#include "pixel.h"
#include "backend.h"
#include "scene.h"

static int (*renderingFunctions[RENDERABLE_COUNT])(Renderer *, Renderable);

int drawRect(Vector2I off, Renderer *r, Frame * src) {
    Frame des = r->frameBuffer;

    //Transform coords on destination (it is only translation so it is easy)
    Vec2f a = (Vec2f){off.x,off.y};
    Vec2f b = (Vec2f){off.x+src->size.x,off.y};
    Vec2f c = (Vec2f){off.x,off.y+src->size.y};
    Vec2f d = (Vec2f){off.x+src->size.x,off.y+src->size.y};

    // .. To find the axis aligned boundig box
    int minX = fmin(fmin(a.x,b.x),fmin(c.x,d.x));
    int minY = fmin(fmin(a.y,b.y),fmin(c.y,d.y));
    int maxX = fmax(fmax(a.x,b.x),fmax(c.x,d.x));
    int maxY = fmax(fmax(a.y,b.y),fmax(c.y,d.y));

    //Then clamp max/min values to destination buffer
    maxX = fmin(des.size.x, fmax(maxX, 0));
    maxY = fmin(des.size.y, fmax(maxY, 0));
    minX = fmin(des.size.x, fmax(minX, 0));
    minY = fmin(des.size.y, fmax(minY, 0));

    if (minX - maxX == 0) return 0; //outside visible frame
    if (maxX - maxY == 0) return 0; //outside visible frame

    for (int y = minY; y < maxY; y++) {
        for (int x = minX; x < maxX; x++) {
            //Transform the coordinate back to sprite space
            Vector2I desPos = {x,y};
            Vector2I srcPosI = {x-off.x,y-off.y};
            Pixel color = frameRead(src, srcPosI);
            frameDraw(&des, desPos, color);
        }
    }

    return 0;
}

int drawRectTransform(Mat3 t, Renderer *r, Frame * src) {
    Frame des = r->frameBuffer;

    Mat3 inv = mat3Inverse(&t);

    // Transform 4 points of frame to frame buffer space
    Vec2f a = (Vec2f){0,0};
    Vec2f b = (Vec2f){src->size.x,0};
    Vec2f c = (Vec2f){0,src->size.y};
    Vec2f d = (Vec2f){src->size.x,src->size.y};

    a = mat3Multiply(&a, &t);
    b = mat3Multiply(&b, &t);
    c = mat3Multiply(&c, &t);
    d = mat3Multiply(&d, &t);

    // .. To find the axis aligned boundig box
    int minX = fmin(fmin(a.x,b.x),fmin(c.x,d.x));
    int minY = fmin(fmin(a.y,b.y),fmin(c.y,d.y));
    int maxX = fmax(fmax(a.x,b.x),fmax(c.x,d.x));
    int maxY = fmax(fmax(a.y,b.y),fmax(c.y,d.y));

    //Then clamp max/min values to destination buffer
    maxX = fmin(des.size.x, fmax(maxX, 0));
    maxY = fmin(des.size.y, fmax(maxY, 0));
    minX = fmin(des.size.x, fmax(minX, 0));
    minY = fmin(des.size.y, fmax(minY, 0));

    if (minX - maxX == 0) return 0; //outside visible frame
    if (minY - maxY == 0) return 0; //outside visible frame

    //Now we can iterate over the pixels of the axis-alignes-bounding-box (AABB) which contain the source frame
    for (int y = minY; y < maxY; y++) {
        for (int x = minX; x < maxX; x++) {
            //Transform the coordinate back to sprite space with the inverse tranform
            Vector2I desPos = {x,y};
            Vec2f desPosF = vecItoF(desPos);
            Vec2f srcPosF = mat3Multiply(&desPosF,&inv);
            Vector2I srcPosI = vecFtoI(srcPosF);

            //TODO: Improve this check by precalculating start/end coord in loop with line intersection
            //We need to check if transformed coord are inside the frame
            //Probably there is a faster way of doing this by checking line intersection
            //with the sprite transformed quad but I don't care for now because we
            //do not use rotations as of now.
            if (srcPosF.x < 0) continue;
            if (srcPosF.y < 0) continue;
            if (srcPosF.x >= src->size.x) continue;
            if (srcPosF.y >= src->size.y) continue;

            Pixel color = frameRead(src, srcPosI);
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
    Sprite * s = ren.impl;
    if (mat3IsOnlyTranslation(&s->t)) {
        Vector2I off = {s->t.elements[2], s->t.elements[5]};
        return drawRect(off,r, &s->frame);
    }

    return drawRectTransform(s->t,r,&s->frame);
};



void renderRenderable(Renderer *r, Renderable ren) {
    renderingFunctions[ren.renderableType](r,ren);
};

int renderScene(Renderer *r, Renderable ren) {
    Scene * s = ren.impl;

    if (!s->visible)
        return 0;

    for (int i = 0; i < s->numberOfRenderables; i++) {
        renderRenderable(r, s->renderables[i]);
    }
    return 0;
};

int rendererInit(Renderer * r, Vector2I size, BackEnd * backEnd) {
    renderingFunctions[RENDERABLE_FRAME] = &renderFrame;
    renderingFunctions[RENDERABLE_SPRITE] = &renderSprite;
    renderingFunctions[RENDERABLE_SCENE] = &renderScene;

    r->scene = 0;
    r->clear = 1;
    r->clearColor = PIXELBLACK;
    r->backEnd = backEnd;

    int e = 0;
    e = frameInit(&(r->frameBuffer), size, backEnd->getFrameBuffer(r, backEnd));
    if (e) return e;

    r->backEnd->init(r, r->backEnd);

    return 0;
}

int rendererRender(Renderer * r)
{
    r->backEnd->beforeRender(r, r->backEnd);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer= r->backEnd->getFrameBuffer(r, r->backEnd);

    //Clear draw buffer before rendering
    Frame des = r->frameBuffer;
    if (r->clear)
        memset(des.frameBuffer,0,des.size.x*des.size.y*sizeof (Pixel));

    renderScene(r, sceneAsRenderable(r->scene));

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




