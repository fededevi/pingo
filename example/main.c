#include "windowbackend.h"
#include "consolebackend.h"
#include "teapot.h"
#include "cube.h"
#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/sprite.h"
#include "../render/scene.h"
#include "../render/object.h"
#include "../render/mesh.h"
#include "../math/mat3.h"

int main(){
    //Enable an example:
    //console_3D_example();
    scene_3D_example();
    //scene_2D_example();
}

int scene_3D_example(){
    Vec2i size = {640, 480};

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

    Object tea;
    tea.mesh = &mesh_teapot;
    sceneAddRenderable(&s, object_as_renderable(&tea));
    tea.material = 0;

    float phi = 0;
    float phi2 = 0;
    Mat4 t;

    while (1) {


        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 2, 16.0,(float)size.x / (float)size.y, 50.0);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0,-9});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //CUBE 1 TRANSFORM - Defines position and orientation of the object
        cube1.transform =  mat4RotateY(phi2 -= 0.08);
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
        t = mat4Translate((Vec3f){0,-0.5,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi += 0.001);

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
