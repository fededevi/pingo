#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/sprite.h"
#include "render/state.h"
#include "test_render_unit.h"
#include <stddef.h>

int test_object_sprite_material(void) {
  printf("Testing Object, Sprite and Material...\n");

  Pixel buf[2 * 2];
  Texture texture;
  texture_init(&texture, (Vec2i){2, 2}, buf);

  Material material;
  TEST_ASSERT(material_init(&material, &texture) == OK, "material init");
  TEST_ASSERT(material.texture == &texture, "material keeps the texture");

  Mesh mesh;
  memset(&mesh, 0, sizeof(mesh));

  Object object;
  TEST_ASSERT(object_init(&object, &mesh, &material) == OK, "object init");
  TEST_ASSERT(object.mesh == &mesh, "mesh kept");
  TEST_ASSERT(object.material == &material, "material kept");
  TEST_ASSERT(object.renderable.render != NULL, "object vtable wired");

  // A mesh with no texture coordinates is drawn untextured rather than
  // dereferencing NULL - three of the four shipped meshes are like this, and
  // it used to crash.
  Object untextured;
  TEST_ASSERT(object_init(&untextured, &mesh, NULL) == OK,
              "object with no material");
  TEST_ASSERT(untextured.material == NULL, "no material kept as NULL");

  Sprite sprite;
  TEST_ASSERT(sprite_init(&sprite, texture) == OK, "sprite init");
  TEST_ASSERT(sprite.texture.pixels == buf, "sprite holds the surface");
  TEST_ASSERT(sprite.renderable.render != NULL, "sprite vtable wired");

  // Every drawable is cast to Renderable * by its address, which is only
  // valid while the embedded member is first. The headers assert this at
  // compile time; assert the consequence here too.
  TEST_ASSERT((void *)&object == (void *)&object.renderable,
              "Object casts to Renderable");
  TEST_ASSERT((void *)&sprite == (void *)&sprite.renderable,
              "Sprite casts to Renderable");

  return 1;
}
