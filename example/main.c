#include "windowbackend.h"
#include "teapot.h"
#include "cube.h"
#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/sprite.h"
#include "../render/scene.h"
#include "../render/object.h"
#include "../render/mesh.h"
#include "../math/mat3.h"
#include <math.h>
#include <string.h>

#define SIZEW 1366
#define SIZEH 768

int main(){
    main_3d_example();
    //main_2d_example();
}

int main_3d_example(){
    Vec2i size = {SIZEW, SIZEH};

    WindowBackEnd backend;
    windowBackEndInit(&backend);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object cube1;
    cube1.mesh = &mesh_cube;
    sceneAddRenderable(&s, object_as_renderable(&cube1));
    cube1.material = 0;

    Object cube2;
    cube2.mesh = &mesh_cube;
    sceneAddRenderable(&s, object_as_renderable(&cube2));

    //TEXTURE FOR CUBE 2
    Texture tex;
    texture_init(&tex, (Vec2i){8,8}, malloc(8*8*sizeof(Pixel)));

    for (int i = 0; i < 8; i++)
        for (int y = 0; y < 8; y++)
            ((uint32_t *)tex.frameBuffer)[i * 8 + y ] = (i + y) % 2 == 0 ? 0xFFFFFFFF : 0x000000FF;

    Material m;
    m.texture = &tex;
    cube2.material = &m;

    Object tea;
    tea.mesh = &mesh_teapot;
    sceneAddRenderable(&s, object_as_renderable(&tea));
    tea.material = 0;

    float phi = 0;
    float phi2 = 0;
    Mat4 t;

    while (1) {
        renderer.camera_projection = mat4Perspective( 2, 8.0,1366.0/768.0, 90.0);

        //VIEW MATRIX
        Mat4 v = mat4Translate((Vec3f) { 0,0,-5});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //CUBE 1 TRANSFORM
        cube1.transform =  mat4RotateY(phi2 -= 0.01);
        t = mat4Scale((Vec3f){1,1,1});
        cube1.transform = mat4MultiplyM(&cube1.transform, &t );
        t = mat4Translate((Vec3f){-3,0.0,0});
        cube1.transform = mat4MultiplyM(&cube1.transform, &t );

        //CUBE 2 TRANSFORM
        cube2.transform =  mat4Translate((Vec3f){0,0.0,0});
        t = mat4Scale((Vec3f){1,1,1});
        cube2.transform = mat4MultiplyM(&cube2.transform, &t );


        //TEA TRANSFORM
        tea.transform = mat4RotateZ(M_PI);
        t =mat4RotateY(phi2);
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Translate((Vec3f){4,-0.5,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi += 0.003);

        rendererSetCamera(&renderer,(Vec4i){0,0,SIZEW,SIZEH});
        rendererRender(&renderer);
    }

    return 0;
}


int main_2d_example(){
    Vec2i size = {SIZEW,SIZEH};

    WindowBackEnd backend;
    windowBackEndInit(&backend);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Texture frame;
    texture_init(&frame, (Vec2i){16,16}, malloc(16*16*sizeof(Pixel)));

    for (int i = 0; i < 16; i++)
        for (int y = 0; y < 16; y++)
            ((Pixel *)frame.frameBuffer)[i * 16 + y ] = (i + y) % 2 == 0
                ? pixelFromUInt8(0x22)
                : pixelFromUInt8(0xBB);

    Sprite sprite;
    spriteInit(&sprite,frame,mat4RotateX(0));

    Scene s;
    sceneInit(&s);
    sceneAddRenderable(&s, spriteAsRenderable(&sprite));

    rendererSetScene(&renderer, &s);

    float phi = 0;
    Mat4 t;
    while (1) {
        sprite.t = mat4Translate((Vec3f){-7.5,-8,0});

        t = mat4RotateZ(phi += 0.01);
        sprite.t = mat4MultiplyM(&sprite.t, &t );

        t = mat4Scale((Vec3f){15,15,1});
        sprite.t = mat4MultiplyM(&sprite.t,&t);

        t = mat4Translate((Vec3f){300,300,0});
        sprite.t = mat4MultiplyM(&sprite.t,&t);

        rendererSetCamera(&renderer,(Vec4i){0,0,SIZEW,SIZEH});
        rendererRender(&renderer);
    }

    return 0;
}

/*
int main2(){
    //Test matrix multiplication
    Mat4 m1= {1,2,1,3,
              6,1,2,4,
              5,7,1,7,
              8,4,2,1};
    Mat4 m2= {1,2,1,3,
              6,1,2,4,
              5,7,1,7,
              8,4,2,1};
    Mat4 m3 = mat4MultiplyM( &m1, &m2);
    m3 = mat4MultiplyM( &m1, &m2);
    m3 = mat4MultiplyM( &m1, &m2);
    m3 = mat4MultiplyM( &m1, &m2);

    //   Expected result:
    //C1	C2	C3	C4
    //1	42	23	12	21
    //2	54	43	18	40
    //3	108	52	34	57
    //4	50	38	20	55
}*/

/*
#include <time.h>
int main3(){
    //Test matrix multiplication speed
    volatile Mat3 m[1000];

    for (int i = 0; i < 1000 * 9 * sizeof (float); i++) {
        *(&m[0].elements[0] + i) = rand();
    }

    clock_t begin = clock();
    for (int i = 0; i < 400000; i++) {
        for (int mc = 0; mc < 1000; mc += 2) {
            m[mc] = mat3MultiplyM(&m[mc],&m[mc+1]);
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf(" time: %f", time_spent);
    return 0;
}
*/
