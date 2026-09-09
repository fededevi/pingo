#include "render/renderer.h"
#include "render/backend.h"
#include "render/depth.h"
#include "render/state.h"
#include "render/transform.h"
#include "test_render_unit.h"

#define W 8
#define H 8

// A backend with no OS behind it, tracking the calls the renderer makes.
typedef struct {
  Backend backend;
  RenderTarget target;
  Pixel colour[W * H];
  PingoDepth depth[W * H];
  int before;
  int after;
  int inits;
} FakeBackend;

static void fb_init(Renderer *r, Backend *b) {
  (void)r;
  ((FakeBackend *)b)->inits++;
}
static void fb_before(Renderer *r, Backend *b) {
  (void)r;
  ((FakeBackend *)b)->before++;
}
static void fb_after(Renderer *r, Backend *b) {
  (void)r;
  ((FakeBackend *)b)->after++;
}
static RenderTarget *fb_target(Renderer *r, Backend *b) {
  (void)r;
  return &((FakeBackend *)b)->target;
}

static void fake_backend_init(FakeBackend *f) {
  memset(f, 0, sizeof(*f));
  f->backend.init = &fb_init;
  f->backend.before_render = &fb_before;
  f->backend.after_render = &fb_after;
  f->backend.get_target = &fb_target;
  render_target_init(&f->target, (Vec2i){W, H}, f->colour, f->depth);
}

int test_renderer(void) {
  printf("Testing Renderer...\n");

  FakeBackend fake;
  fake_backend_init(&fake);

  Renderer r;
  TEST_ASSERT(renderer_init(&r, (Vec2i){W, H}, (Backend *)&fake) == OK,
              "init");
  TEST_ASSERT_EQ_INT(1, fake.inits, "backend init called once");
  TEST_ASSERT(r.target.color.pixels == fake.colour, "target came from backend");
  TEST_ASSERT(r.clear, "clearing on by default");
  TEST_ASSERT(r.enable_backface_culling, "backface culling on by default");

  TEST_ASSERT(renderer_init(NULL, (Vec2i){W, H}, (Backend *)&fake) != OK,
              "NULL renderer rejected");
  TEST_ASSERT(renderer_init(&r, (Vec2i){W, H}, NULL) != OK,
              "NULL backend rejected");

  fake_backend_init(&fake);
  renderer_init(&r, (Vec2i){W, H}, (Backend *)&fake);

  // No root: rendering must fail rather than dereference it.
  TEST_ASSERT(renderer_render(&r) != OK, "no root is an error, not a crash");

  Transform root;
  transform_init(&root, NULL, mat4Identity());
  TEST_ASSERT(renderer_set_root_renderable(&r, (Renderable *)&root) == OK,
              "set root");
  TEST_ASSERT(renderer_set_root_renderable(&r, NULL) != OK,
              "NULL root rejected");
  renderer_set_root_renderable(&r, (Renderable *)&root);

  // clear_color is honoured. It used to be ignored: the clear was a memset to
  // zero, so asking for anything but black did nothing.
  r.clear = true;
  r.clear_color = pixel_from_rgba(10, 20, 30, 255);
  for (int i = 0; i < W * H; i++) {
    fake.colour[i] = pixel_from_uint8(99);
    fake.depth[i].d = 123;
  }
  TEST_ASSERT(renderer_render(&r) == OK, "render");
  TEST_ASSERT_EQ_INT(1, fake.before, "before_render called");
  TEST_ASSERT_EQ_INT(1, fake.after, "after_render called");
  const Pixel expected = pixel_from_rgba(10, 20, 30, 255);
  TEST_ASSERT(memcmp(&fake.colour[0], &expected, sizeof(Pixel)) == 0,
              "cleared to clear_color, not to black");
  TEST_ASSERT(memcmp(&fake.colour[W * H - 1], &expected, sizeof(Pixel)) == 0,
              "the whole surface was cleared");
  TEST_ASSERT_EQ_INT(0, fake.depth[0].d, "depth cleared");
  TEST_ASSERT_EQ_INT(0, fake.depth[W * H - 1].d, "all of depth cleared");

  // clear = false leaves the colour surface alone, which is what a backend
  // compositing over its own content needs.
  for (int i = 0; i < W * H; i++) {
    fake.colour[i] = pixel_from_uint8(77);
  }
  r.clear = false;
  TEST_ASSERT(renderer_render(&r) == OK, "render without clearing");
  TEST_ASSERT_EQ_INT(77, pixel_to_uint8(&fake.colour[0]),
                     "clear = false preserved the surface");

  return 1;
}
