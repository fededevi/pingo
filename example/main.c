#include "windowbackend.h"
#include "consolebackend.h"
#include "teapot.h"
#include "cube.h"
#include "pingo_mesh.h"
#include "viking.h"
#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/sprite.h"
#include "../render/scene.h"
#include "../render/object.h"
#include "../render/mesh.h"
#include "../math/mat3.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(){
    //Enable an example:
    simpleModel();
    //console_3D_example();
    //scene_3D_example();
    //scene_2D_example();
}

int simpleModel(){
    Vec2i size = {1280, 800};

    WindowBackEnd backend;
    windowBackEndInit(&backend, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object viking_room;
    viking_room.mesh = &viking_mesh;
    sceneAddRenderable(&s, object_as_renderable(&viking_room));
    viking_room.material = 0;

    Pixel * image = malloc(1024*1024*4);

    FILE * file   = fopen("C:/Users/fededevi/Desktop/room/source/vikingroom/texture.data", "rb"); // read only
    if(!file)
    {
        printf("We are unable to read the file!");
    }else if(file)
    {
        printf("The file has successfully loaded!");
    }

    for (int i = 1023; i > 0; i--) {
    for (int j = 0; j < 1024; j++) {
            uint8_t v1;
            uint8_t v2;
            uint8_t v3;
            uint8_t v4;
            fread(&v1, 1, 1, file);
            fread(&v2, 1, 1, file);
            fread(&v3, 1, 1, file);
            fread(&v4, 1, 1, file);
            image[i*1024 + j].r = v1;
            image[i*1024 + j].g = v2;
            image[i*1024 + j].b = v3;
            image[i*1024 + j].a = v4;
        }
    }

    fclose(file);

    Texture tex;
    texture_init(&tex, (Vec2i){1024,1024},image);


    Material m;
    m.texture = &tex;
    viking_room.material = &m;

    float phi = 0;
    Mat4 t;

    while (1) {
        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 25.0,(float)size.x / (float)size.y, 70.0);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0,-7});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM - Defines position and orientation of the object
        viking_room.transform = mat4RotateZ(3.142128);
        t = mat4Scale((Vec3f){0.2,0.2,0.2});
        viking_room.transform = mat4MultiplyM(&viking_room.transform, &t );
        t = mat4Translate((Vec3f){0,1,0});
        viking_room.transform = mat4MultiplyM(&viking_room.transform, &t );
        t = mat4RotateZ(3.1421);
        viking_room.transform = mat4MultiplyM(&viking_room.transform, &t );

        //SCENE
        s.transform = mat4RotateY(sin(phi -= 0.005));

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
        rendererRender(&renderer);

    }

    return 0;
}

int scene_3D_example(){
    Vec2i size = {1280, 800};

    WindowBackEnd backend;
    windowBackEndInit(&backend, size);

    //ConsoleBackend backend;
    //console_backend_init(&backend, size);

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
            ((uint32_t *)tex.frameBuffer)[i * 8 + y ] = (i + y) % 2 == 0 ? 0xFF00FFFF : 0x000000FF;

    Material m;
    m.texture = &tex;
    cube2.material = &m;
    cube1.material = &m;

    Object tea;
    tea.mesh = &pingo_mesh;
    sceneAddRenderable(&s, object_as_renderable(&tea));
    tea.material = 0;

    float phi = 0;
    float phi2 = 0;
    Mat4 t;

    while (1) {


        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 25.0,(float)size.x / (float)size.y, 70.0);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0,-10});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //CUBE 1 TRANSFORM - Defines position and orientation of the object
        cube1.transform =  mat4RotateY(phi2 -= 0.01);
        t = mat4Scale((Vec3f){1,1,1});
        cube1.transform = mat4MultiplyM(&cube1.transform, &t );
        t = mat4Translate((Vec3f){-3,0.0,0});
        cube1.transform = mat4MultiplyM(&cube1.transform, &t );

        //CUBE 2 TRANSFORM - Defines position and orientation of the object
        cube2.transform =  mat4Translate((Vec3f){3,0.0,0});
        t = mat4Scale((Vec3f){1,1,1});
        cube2.transform = mat4MultiplyM(&cube2.transform, &t );


        //TEA TRANSFORM - Defines position and orientation of the object
        tea.transform = mat4RotateZ(3.142128);
        t =mat4RotateY(phi2);
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Scale((Vec3f){0.02,0.02,0.02});
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Translate((Vec3f){0,1,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4RotateZ(3.1421);
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi -= 0.005);

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
        rendererRender(&renderer);

    }

    return 0;
}

int console_3D_example(){
    Vec2i size = {200, 70};

    ConsoleBackend backend;
    console_backend_init(&backend, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );
    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object tea;
    tea.mesh = &mesh_teapot;
    sceneAddRenderable(&s, object_as_renderable(&tea));
    tea.material = 0;

    float phi = 0;
    Mat4 t;

    while (1) {
        renderer.camera_projection = mat4Perspective( 2, 16.0,(float)size.x / (float)size.y, 45.0);

        //VIEW MATRIX
        Mat4 v = mat4Translate((Vec3f) { 0,0,-9});
        Mat4 rotateDown = mat4RotateX(0.40);
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM
        tea.transform = mat4RotateZ(3.142128);
        t = mat4Translate((Vec3f){0,-0.5,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi += 0.003);

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
        rendererRender(&renderer);
    }
}

int scene_2D_example(){
    Vec2i size = {800,600};

    WindowBackEnd backend;
    windowBackEndInit(&backend, size);

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

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
        rendererRender(&renderer);
    }

    return 0;
}
