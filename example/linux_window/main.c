
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "math/mat4.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/entity.h"

#include "linux_window_backend.h"

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

    Pixel * image = loadTexture("assets/viking.rgba", (Vec2i){1024,1024});

    Texture texture;
    texture_init(&texture, (Vec2i){1024, 1024}, image);

    Material material;
    material_init(&material, &texture);

    Object object;
    object_init(&object, &viking_mesh, &material);

    Entity root_entity;
    entity_init(&root_entity, (Renderable*)&object, mat4Identity());


    Vec2i size = {640, 480};
    LinuxWindowBackend backend;
    linuxWindowBackendInit(&backend, size);

    Renderer renderer;
    renderer_init(&renderer, size, &backend );
    renderer_set_root_renderable(&renderer, (Renderable*)&root_entity);

    float phi = 0;
    Mat4 t;

    renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);

    Mat4 translate_back = mat4Translate((Vec3f) { 0,0,-35});
    Mat4 rotate_down = mat4RotateX(0.40);
    renderer.camera_view = mat4MultiplyM(&rotate_down, &translate_back );

    while (1) {

        //SCENE
        root_entity.transform = mat4RotateY(phi);
        phi += 0.01;

        renderer_render(&renderer);
	}

    return 0;
}
