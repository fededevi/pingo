#pragma once

#include "renderable.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * A transform with something under it.
 *
 * `content` is what this node draws and may be NULL, which makes the entity a
 * pure grouping node - a transform applied to its children and nothing else.
 * Every scene graph needs those, and requiring content meant wrapping a
 * transform around a dummy just to group.
 *
 * `children` is an array of Renderable pointers, the same type as `content`,
 * so any drawable can be a child. It used to be a contiguous Entity array
 * behind an untyped Array, which meant an Object had to be wrapped in an
 * Entity before it could be a child.
 *
 * Neither array nor content is owned: the caller keeps them alive.
 */
typedef struct {
  Renderable renderable;
  Renderable *content;
  Mat4 transform;
  bool visible;
  Renderable **children;
  size_t child_count;
} Entity;

extern int entity_init(Entity *this, Renderable *renderable, Mat4 transform);

extern int entity_init_children(Entity *this, Renderable *renderable,
                                Mat4 transform, Renderable **children,
                                size_t child_count);
