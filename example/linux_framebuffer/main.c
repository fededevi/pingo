
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "math/mat4.h"
#include "render/entity.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/pixel.h"
#include "render/renderable.h"
#include "render/renderer.h"

#include "linux_framebuffer_backend.h"

#include "assets/viking.h"


Pixel * loadTexture(char * filename, Vec2i size) {
    Pixel * image = malloc(size.x*size.y*4);
    FILE * file   = fopen(filename, "rb");
    if (file == 0) {
        printf("Error: Could not open file %s\n", filename);
        exit(-1);
    }
    for (int i = 1023; i > 0; i--) {
    for (int j = 0; j < 1024; j++) {
            char r, g, b, a;
            fread(&r, 1, 1, file);
            fread(&g, 1, 1, file);
            fread(&b, 1, 1, file);
            fread(&a, 1, 1, file);
            image[i*1024 + j] = pixelFromRGBA(r, g, b, a);
        }
    }
    fclose(file);
    return image;
}

int main(){

    Pixel * image = loadTexture("./viking.rgba", (Vec2i){1024,1024});
    Texture texture;
    texture_init(&texture, (Vec2i){1024, 1024}, image);

    Material material;
    material_init(&material, &texture);

    Object viking_object;
    object_init(&viking_object, &viking_mesh, &material);

    Entity root_entity;
    entity_init(&root_entity, (Renderable*)&viking_object, mat4Identity());


    Vec2i size = {1376, 768};
    LinuxFramebufferBackend backend;
    linux_framebuffer_backend_init(&backend, size, "/dev/fb0");

    Renderer renderer;
    renderer_init(&renderer, size,(Backend*) &backend );
    renderer_set_root_renderable(&renderer, (Renderable*)&root_entity);

    float phi = 0;
    Mat4 t;

	while (1) {
        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,2,-35});

        Mat4 rotateDown = mat4RotateX(-0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM - Defines position and orientation of the object
        root_entity.transform = mat4RotateZ(3.142128);
        t = mat4RotateZ(0);
        root_entity.transform = mat4MultiplyM(&root_entity.transform, &t );

        //SCENE
        root_entity.transform = mat4RotateY(phi);
        phi += 0.01;

        renderer_render(&renderer);
        usleep(40000);
	}

    return 0;
}
