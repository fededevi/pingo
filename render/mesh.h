#pragma once

#include <stdint.h>

#include "../math/vec3.h"

#include "material.h"

typedef struct Mesh {
    int vertices_count;
    Vec3f * vertices;

    int triangles_count;
    uint32_t * triangles;
} Mesh;

Mesh mesh_test();

