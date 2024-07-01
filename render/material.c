#include "material.h"
#include "render/state.h"

int material_init(Material *this, Texture *texture)
{
    IF_NULL_RETURN(this, INIT_ERROR);
    IF_NULL_RETURN(texture, INIT_ERROR);

    this->texture = texture;
    return OK;
}
