#include "example/terminal/terminalbackend.h"
#include "math/mat4.h"
#include "assets/viking.h"
#include "terminalbackend.h"

#include "render/entity.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/pixel.h"
#include "render/renderer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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


    Vec2i size = {160, 120};
    TerminalBackend backend;
    terminal_backend_init(&backend, size);

    Renderer renderer;
    renderer_init(&renderer, size, (Backend*)&backend );
    renderer_set_root_renderable(&renderer, (Renderable*)&root_entity);

    float phi = 0;
    Mat4 t;

    renderer.camera_projection = mat4Perspective(3, 50.0, (float) size.x / (float) size.y, 0.1);

    renderer.camera_view = mat4Translate((Vec3f){0, 0, 0});

    while (1) {
        // Rotation around Y-axis
        Mat4 rotation = mat4RotateY(phi);

        // Move object back so it's visible
        Mat4 translation = mat4Translate((Vec3f){0, -7, -50});

        // Combine transforms: T * R * Flip
        //Mat4 model = mat4MultiplyM(&rotation, &flip);
        Mat4 model = mat4MultiplyM(&rotation, &translation);

        root_entity.transform = model;

        renderer_render(&renderer);

        phi += 0.01f;
        usleep(16000); // ~60 FPS
    }

    return 0;
}

