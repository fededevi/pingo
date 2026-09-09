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
 * tree, since an Transform is itself a Renderable and so can be a child.
 *
 * There is one list, `children`. An earlier version had both a single
 * `content` and a separate list, because the list could only hold Entities
 * and so could not hold an Object directly; now that it holds Renderable
 * pointers the two were the same mechanism twice. `one` is storage for the
 * single-child case so that transform_init does not force the caller to declare
 * a one-element array for what is by far the common use.
 *
 * Nothing here is owned: the caller keeps the children alive.
 */
typedef struct Transform {
  // Must stay first: callers cast an Transform * to Renderable *.
  Renderable renderable;

  /** This node's own transform, concatenated with its parent's. */
  Mat4 local;
  bool visible;

  Renderable **children;
  size_t child_count;

  Renderable *one[1];
} Transform;

PINGO_ASSERT_RENDERABLE_FIRST(Transform);

/** The vtable entry, declared so it can be called and tested
 * directly and not only through Renderable.render. */
extern int transform_render(void *this, Mat4 parent, Renderer *renderer);

/** One child, stored inline. `renderable` may be NULL for a bare group. */
extern int transform_init(Transform *this, Renderable *renderable, Mat4 local);

/** Any number of children, whose array the caller owns. */
extern int transform_init_children(Transform *this, Mat4 local,
                                Renderable **children, size_t child_count);
