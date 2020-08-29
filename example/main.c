#include "windowbackend.h"
#include "../render/renderer.h"
#include "../render/frame.h"
#include "../render/sprite.h"
#include "../render/scene.h"
#include "../math/mat3.h"
#include <math.h>
    #define SIZE 1024
int main(){
    Pixel fb[1];
    Vec2i pos = {0,0};
    Vec2i size = {SIZE,SIZE};

    WindowBackEnd backend;
    windowBackEndInit(&backend, pos, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Frame frame;
    frameInit(&frame,(Vec2i){SIZE,SIZE},fb);
    Sprite sprite;
    spriteInit(&sprite,frame,mat3Rotate(45));

    Scene s;
    sceneInit(&s);
    sceneAddRenderable(&s, spriteAsRenderable(&sprite));

    rendererSetScene(&renderer, &s);


    Pixel fb2[256] = {PIXELWHITE};
    memset(&fb2, 255, 256);
    Frame frame2;
    frameInit(&frame2,(Vec2i){16,16},fb2);
    Sprite sprite2;
    spriteInit(&sprite2,frame2,mat3Rotate(45));
    sceneAddRenderable(&s, spriteAsRenderable(&sprite2));

    float y;
    float i[256];
    while (1) {
        y += 0.01;
        i[rand() % 256] = rand();
        Mat3 m1 = mat3Rotate(i[0]);
        Mat3 m2 = mat3Scale((Vec2f){2,2});
        m1 = mat3MultiplyM(&m1,&m2);
        m2 = mat3Translate((Vec2f){300,300});
        m1 = mat3MultiplyM(&m1,&m2);
        sprite.t = mat3Scale((Vec2f){1,1});


        sprite2.t = mat3Translate((Vec2f){128 * (1 + sin(y)),16 *(1 - cos(y))});
        rendererRender(&renderer);
    }

    return 0;
}

