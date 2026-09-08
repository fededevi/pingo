#pragma once

/**
 * Single home for the typedef of every struct that is forward-declared
 * elsewhere in the render headers.
 *
 * Repeating `typedef struct X X;` is a constraint violation in C99 and only
 * became legal in C11, so each name is typedef'd here exactly once and the
 * header that defines the struct writes a plain `struct X { ... };`.
 */

typedef struct Backend Backend;
typedef struct Material Material;
typedef struct Mesh Mesh;
typedef struct PingoDepth PingoDepth;
typedef struct Pixel Pixel;
typedef struct Renderer Renderer;
typedef struct Texture Texture;
