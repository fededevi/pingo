#include "sprite.h"
#include "math/mat4.h"
#include "render/rasterizer.h"
#include "renderer.h"
#include "state.h"

int render_sprite(void *_sprite, Mat4 transform, Renderer *renderer)
{
    IF_NULL_RETURN(_sprite, INIT_ERROR);
    IF_NULL_RETURN(renderer, INIT_ERROR);

    Sprite *sprite = _sprite;

/*
 *TODO: Implement for 3D?
 * if (mat4IsOnlyTranslation(&t)) {
 *     Vec2i off = {s->t.elements[2], s->t.elements[5]};
 *     rasterizer_draw_pixel_perfect(off,r, &s->frame);
 *     s->t = backUp;
 *     return 0;
 * }
*/
    rasterizer_draw_transformed(transform, renderer, &sprite->texture);
    return OK;
};

int sprite_init(Sprite *this, Texture texture)
{
    IF_NULL_RETURN(this, INIT_ERROR);

    this->texture = texture;
    this->renderable.render = &render_sprite;

  return OK;
}

int sprite_randomize(Sprite *this) {
  for (int x = 0; x < this->texture.size.x; x++)
    for (int y = 0; y < this->texture.size.y; y++)
      texture_draw(&this->texture, (Vec2i){x, y}, pixelRandom());

  return OK;
}
