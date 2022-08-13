#include "terminalbackend.h"
#include "../render/mesh.h"
#include "../render/object.h"
#include "../render/pixel.h"
#include "../render/renderer.h"
#include "../render/scene.h"
#include "../render/texture.h"
#include "pingo_mesh.h"
#include "teapot.h"
#include "viking.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Pixel * loadTexture(char * filename, Vec2i size) {
    //Load from filesystem from a RAW RGBA file
    Pixel * image = malloc(size.x*size.y*4);
    FILE * file   = fopen(filename, "rb");
    if (file == 0) {
        printf("Error: Could not open file %s\n", filename);
        exit(-1);
    }
    for (int i = 1023; i > 0; i--) {
    for (int j = 0; j < 1024; j++) {
            fread(&image[i*1024 + j].r, 1, 1, file);
            fread(&image[i*1024 + j].g, 1, 1, file);
            fread(&image[i*1024 + j].b, 1, 1, file);
            fread(&image[i*1024 + j].a, 1, 1, file);
        }
    }
    fclose(file);
    return image;
}

int main(){
    Vec2i size = {160, 40};

    TerminalBackend backend;
    terminal_backend_init(&backend, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object object;
    object.mesh = &mesh_teapot;
    object.material = 0;
    sceneAddRenderable(&s, object_as_renderable(&object));

    float phi = 0;
    Mat4 t;

	// optional hide cursor
	/*printf("\033[?25l");*/
	while (1) {
        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0.7,-3});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM - Defines position and orientation of the object
        object.transform = mat4RotateZ(3.142128);
        t = mat4Scale((Vec3f){1,1,1});
        object.transform = mat4MultiplyM(&object.transform, &t );
        t = mat4Translate((Vec3f){0,-1.4,0});
        object.transform = mat4MultiplyM(&object.transform, &t );
        t = mat4RotateZ(0);
        object.transform = mat4MultiplyM(&object.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi);
        phi += 0.01;

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
        rendererRender(&renderer);
        usleep(40000);
	}

    return 0;
}

