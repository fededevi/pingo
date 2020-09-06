#pragma once

#include <stdint.h>

#include "../math/vec3.h"

#include "material.h"

typedef struct Mesh {
    int indexes_count;
    uint32_t * indexes;
    Vec3f * positions;
    Vec2f * textCoord;
} Mesh;


