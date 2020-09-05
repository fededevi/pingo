#include <string.h>
#include <stdio.h>

#include "renderer.h"
#include "sprite.h"
#include "pixel.h"
#include "backend.h"
#include "scene.h"
#include "rasterizer.h"
#include "object.h"


int renderFrame(Renderer *r, Renderable ren) {
    Texture * f = ren.impl;
    return rasterizer_draw_pixel_perfect((Vec2i){0,0},r,f);
};

int renderSprite(Mat4 transform, Renderer *r, Renderable ren) {
    Sprite * s = ren.impl;
    Mat4 backUp = s->t;

    //Apply parent transform to the local transform
    s->t = mat4MultiplyM( &s->t, &transform);

    //Apply camera translation
    Mat4 newMat = mat4Translate((Vec3f){-r->camera.x,-r->camera.y,0});
    s->t = mat4MultiplyM(&s->t, &newMat);
    /*
    if (mat4IsOnlyTranslation(&s->t)) {
        Vec2i off = {s->t.elements[2], s->t.elements[5]};
        rasterizer_draw_pixel_perfect(off,r, &s->frame);
        s->t = backUp;
        return 0;
    }

    if (mat4IsOnlyTranslationDoubled(&s->t)) {
        Vec2i off = {s->t.elements[2], s->t.elements[5]};
        rasterizer_draw_pixel_perfect_doubled(off,r, &s->frame);
        s->t = backUp;
        return 0;
    }*/

    rasterizer_draw_transformed(s->t,r,&s->frame);
    s->t = backUp;
    return 0;
};



void renderRenderable(Mat4 transform, Renderer *r, Renderable ren) {
    renderingFunctions[ren.renderableType](transform, r, ren);
};

int renderScene(Mat4 transform, Renderer *r, Renderable ren) {
    Scene * s = ren.impl;
    if (!s->visible)
        return 0;

    //Apply hierarchy transfom
    Mat4 newTransform = mat4MultiplyM(&s->transform,&transform);
    for (int i = 0; i < s->numberOfRenderables; i++) {
        renderRenderable(newTransform, r, s->renderables[i]);
    }
    return 0;
};

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

float edgeFunction(const Vec3f * a, const Vec3f * b, const Vec3f* c)
{
    return (c->x - a->x) * (b->y - a->y) - (c->y - a->y) * (b->x - a->x);
}

float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3) {
    return (y2 - y1)*(x3 - x2) - (y3 - y2)*(x2 - x1);
}

int renderObject(Mat4 object_transform, Renderer *r, Renderable ren) {
    Object * o = ren.impl;
    //Apply model transform to hierarchy transform
    Mat4 model = mat4MultiplyM(&o->transform,&object_transform);
    Mat4 modelview = mat4MultiplyM(  &model, &r->camera_transform);
    //Apply camera transform

    for (int i = 0; i < o->mesh.vertices_count; i+=3) {
        Vec3f a = mat4MultiplyVec3(&o->mesh.vertices[i+0], &modelview);
        Vec3f b = mat4MultiplyVec3(&o->mesh.vertices[i+1], &modelview);
        Vec3f c = mat4MultiplyVec3(&o->mesh.vertices[i+2], &modelview);

        if (isClockWise(a.x,a.y,b.x,b.y,c.x,c.y) < 0)
            continue;

        float minX = MIN(MIN(a.x,b.x),c.x);
        float minY = MIN(MIN(a.y,b.y),c.y);
        float maxX = MAX(MAX(a.x,b.x),c.x);
        float maxY = MAX(MAX(a.y,b.y),c.y);

        for (int y = minY; y < maxY; y++) {
            for (int x = minX; x < maxX; x++) {
                //Transform the coordinate back to sprite space
                Vec2i desPos = {x,y};
                Vec3f desPosF = {x,y,1};

                // area of the triangle multiplied by 2
                float area = edgeFunction(&a, &b, &c);

                //Area of sub triangles
                float j0 = edgeFunction(&a, &b, &desPosF) / area;
                float j1 = edgeFunction(&b, &c, &desPosF) / area;
                float j2 = 1.0 - j0 - j1;//edgeFunction(&c, &a, &desPosF) / area;

                // If all the areas are positive then pèoint is inside triangle
                if (j0 < 0 ||j1 < 0 ||j2 < 0)
                    continue;

                float depth =  1/(j0/a.z  + j1/b.z  + j2/c.z);

                if (zbuffer[x][y] < depth)
                    continue;

                // barycentric coordinates are the areas of the sub-triangles divided by the area of the main triangle
                texture_draw(&r->frameBuffer, desPos, pixelFromRGBA(depth,depth,depth,255));
                //texture_draw(&r->frameBuffer, desPos, pixelFromRGBA(j0*255,j1*255,j2*255,255));
                zbuffer[x][y] = depth;
            }
        }

        // texture_draw(&r->frameBuffer, (Vec2i){a.x,a.y}, pixelFromUInt8(100));
    }

    return 0;
};

int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd) {
    renderingFunctions[RENDERABLE_SPRITE] = &renderSprite;
    renderingFunctions[RENDERABLE_SCENE] = &renderScene;
    renderingFunctions[RENDERABLE_OBJECT] = &renderObject;

    r->scene = 0;
    r->clear = 1;
    r->clearColor = PIXELBLACK;
    r->backEnd = backEnd;

    r->backEnd->init(r, r->backEnd, (Vec4i){0,0,0,0});

    int e = 0;
    e = texture_init(&(r->frameBuffer), size, backEnd->getFrameBuffer(r, backEnd));
    if (e) return e;


    return 0;
}

/**
 * @brief Clear the whole framebuffer slowly. This prevents underrun in the FPGA draw buffer and prevents desynchronization
 * of the video signal
 */
void clearBufferSlowly (Texture f)
{
    int length = f.size.x*1;

    for (int y = 0; y < f.size.y; y++) {
        memset(f.frameBuffer+length*y,0,length*4);
    }
}


int rendererRender(Renderer * r)
{
    for (int i = 1366*768; i > 0; --i){
        zbuffer[0][i] = 200000;
    }

    r->backEnd->beforeRender(r, r->backEnd);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer= r->backEnd->getFrameBuffer(r, r->backEnd);

    //Clear draw buffer before rendering
    if (r->clear) {
        clearBufferSlowly(r->frameBuffer);
    }

    renderScene(mat4Identity(), r, sceneAsRenderable(r->scene));

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
