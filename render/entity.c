#include "entity.h"
#include "math/mat4.h"
#include "render/array.h"
#include "state.h"
#include <stddef.h>

int entity_render(void *this, Mat4 transform, Renderer *renderer)
{
    Entity *entity = this;
    IF_NULL_RETURN(entity, RENDER_ERROR);
    IF_NULL_RETURN(renderer, RENDER_ERROR);

    if (!entity->visible)
        return OK;

    Mat4 new_transform = mat4MultiplyM(&entity->transform, &transform);

    for (int i = 0; i < entity->children_entities.size; i++) {
        Entity *child_entity = &entity->children_entities.data[i];
        child_entity->renderable.render(child_entity, new_transform, renderer);
    }

    Renderable *renderable = entity->entity_renderable;

    return renderable->render(renderable, new_transform, renderer);
};

int entity_init(Entity *this, Renderable *renderable, Mat4 transform)
{
    IF_NULL_RETURN(this, INIT_ERROR);
    IF_NULL_RETURN(renderable, INIT_ERROR);

    this->entity_renderable = renderable;
    this->renderable.render = &entity_render;
    this->transform = transform;
    this->visible = true;

    array_init(&this->children_entities, 0, 0);

    return OK;
}

int entity_init_children(Entity *this, Renderable *renderable, Mat4 transform, Entity children[], size_t children_count)
{
    IF_NULL_RETURN(this, INIT_ERROR);
    IF_NULL_RETURN(renderable, INIT_ERROR);

    this->entity_renderable = renderable;
    this->renderable.render = &entity_render;
    this->transform = transform;
    this->visible = true;

    array_init(&this->children_entities, children_count, children);

    return OK;
}
