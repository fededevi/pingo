#include "transform.h"
#include "math/mat4.h"
#include "state.h"
#include <stddef.h>

static int transform_render(void *this, Mat4 parent, Renderer *renderer) {
  Transform *node = this;
  IF_NULL_RETURN(node, RENDER_ERROR);
  IF_NULL_RETURN(renderer, RENDER_ERROR);

  if (!node->visible)
    return OK;

  Mat4 world = mat4MultiplyM(&node->local, &parent);

  // One loop, because there is one list. This used to draw the children and
  // then a separate `content` through identical code.
  for (size_t i = 0; i < node->child_count; i++) {
    Renderable *child = node->children[i];
    if (child == NULL) {
      continue;
    }
    const int e = child->render(child, world, renderer);
    if (e != OK) {
      return e;
    }
  }

  return OK;
};

int transform_init(Transform *this, Renderable *renderable, Mat4 local) {
  IF_NULL_RETURN(this, INIT_ERROR);

  // NULL is a group with nothing of its own, which is a legitimate node.
  this->one[0] = renderable;
  const int e = transform_init_children(this, local, this->one,
                                     (renderable == NULL) ? 0 : 1);
  return e;
}

int transform_init_children(Transform *this, Mat4 local,
                            Renderable **children, size_t child_count) {
  IF_NULL_RETURN(this, INIT_ERROR);

  this->renderable.render = &transform_render;
  this->local = local;
  this->visible = true;
  this->children = children;
  this->child_count = (children == NULL) ? 0 : child_count;

  return OK;
}
