#include "linuxtermbackend.h"
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

#include "../render/pixel.h"
#include "../render/depth.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
	Vec2i size = {120, 60};

	LinuxTermBackend backend;
	linuxterm_backend_init(&backend, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object viking_room;
	viking_room.mesh = &viking_mesh;
    sceneAddRenderable(&s, object_as_renderable(&viking_room));
    viking_room.material = 0;

	Pixel * image = loadTexture("../example/texture.data", (Vec2i){1024,1024});

	Texture tex;
	texture_init(&tex, (Vec2i){1024, 1024},image);

	Material m;
	m.texture = &tex;
	viking_room.material = &m;

    float phi = 0;
    Mat4 t;

	// optional hide cursor
	/*printf("\033[?25l");*/
	while (1) {
        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 250.0,(float)size.x / (float)size.y, 70.0);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0.7,-7});
        Mat4 rotateDown = mat4RotateX(-0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM - Defines position and orientation of the object
        viking_room.transform = mat4RotateZ(3.142128);
        t = mat4Scale((Vec3f){0.2,0.2,0.2});
        viking_room.transform = mat4MultiplyM(&viking_room.transform, &t );
        t = mat4Translate((Vec3f){0,0,0});
        viking_room.transform = mat4MultiplyM(&viking_room.transform, &t );
        t = mat4RotateZ(0);
        viking_room.transform = mat4MultiplyM(&viking_room.transform, &t );

        //SCENE
        s.transform = mat4RotateY(cos(phi -= 0.05)+0.64);

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
		rendererRender(&renderer);

		// lower the frame rate
		usleep(50000);

	}

    return 0;
}

