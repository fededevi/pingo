#pragma once

#include "renderable.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * A transform applied to whatever hangs under it, with a visibility toggle.
 *
 * That is its whole job: an Object is a mesh and a material and has no
 * position, a Sprite is a texture and has no position, and this is what gives
 * either one a place in the world. Composing these is what makes the scene a
 * tree, since an Entity is itself a Renderable and so can be a child.
 *
 * There is one list, `children`. An earlier version had both a single
 * `content` and a separate list, because the list could only hold Entities
 * and so could not hold an Object directly; now that it holds Renderable
 * pointers the two were the same mechanism twice. `one` is storage for the
 * single-child case so that entity_init does not force the caller to declare
 * a one-element array for what is by far the common use.
 *
 * Nothing here is owned: the caller keeps the children alive.
 */
typedef struct Entity {
  // Must stay first: callers cast an Entity * to Renderable *.
  Renderable renderable;

  Mat4 transform;
  bool visible;

  Renderable **children;
  size_t child_count;

  Renderable *one[1];
} Entity;

PINGO_ASSERT_RENDERABLE_FIRST(Entity);

/** One child, stored inline. `renderable` may be NULL for a bare group. */
extern int entity_init(Entity *this, Renderable *renderable, Mat4 transform);

/** Any number of children, whose array the caller owns. */
extern int entity_init_children(Entity *this, Mat4 transform,
                                Renderable **children, size_t child_count);
