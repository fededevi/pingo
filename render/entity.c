#include "entity.h"
#include "math/mat4.h"
#include "state.h"
#include <stddef.h>

int entity_render(void *this, Mat4 transform, Renderer *renderer) {
  Entity *entity = this;
  IF_NULL_RETURN(entity, RENDER_ERROR);
  IF_NULL_RETURN(renderer, RENDER_ERROR);

  if (!entity->visible)
    return OK;

  Mat4 new_transform = mat4MultiplyM(&entity->transform, &transform);

  // One loop, because there is one list. This used to draw the children and
  // then a separate `content` through identical code.
  for (size_t i = 0; i < entity->child_count; i++) {
    Renderable *child = entity->children[i];
    if (child == NULL) {
      continue;
    }
    const int e = child->render(child, new_transform, renderer);
    if (e != OK) {
      return e;
    }
  }

  return OK;
};

int entity_init(Entity *this, Renderable *renderable, Mat4 transform) {
  IF_NULL_RETURN(this, INIT_ERROR);

  // NULL is a group with nothing of its own, which is a legitimate node.
  this->one[0] = renderable;
  const int e = entity_init_children(this, transform, this->one,
                                     (renderable == NULL) ? 0 : 1);
  return e;
}

int entity_init_children(Entity *this, Mat4 transform, Renderable **children,
                         size_t child_count) {
  IF_NULL_RETURN(this, INIT_ERROR);

  this->renderable.render = &entity_render;
  this->transform = transform;
  this->visible = true;
  this->children = children;
  this->child_count = (children == NULL) ? 0 : child_count;

  return OK;
}
