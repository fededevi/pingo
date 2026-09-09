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

  for (size_t i = 0; i < entity->child_count; i++) {
    Renderable *child = entity->children[i];
    if (child != NULL) {
      const int e = child->render(child, new_transform, renderer);
      if (e != OK) {
        return e;
      }
    }
  }

  // A grouping node has no content of its own. This used to be dereferenced
  // unconditionally, so a zeroed entity crashed rather than drawing nothing.
  if (entity->content == NULL) {
    return OK;
  }

  return entity->content->render(entity->content, new_transform, renderer);
};

int entity_init(Entity *this, Renderable *renderable, Mat4 transform) {
  return entity_init_children(this, renderable, transform, NULL, 0);
}

int entity_init_children(Entity *this, Renderable *renderable, Mat4 transform,
                         Renderable **children, size_t child_count) {
  IF_NULL_RETURN(this, INIT_ERROR);

  // renderable may be NULL: that is a grouping node.
  this->content = renderable;
  this->renderable.render = &entity_render;
  this->transform = transform;
  this->visible = true;
  this->children = children;
  this->child_count = (children == NULL) ? 0 : child_count;

  return OK;
}
