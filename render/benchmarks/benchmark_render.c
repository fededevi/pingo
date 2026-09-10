/**
 * Render benchmark: renders a fixed number of frames and times them.
 *
 * A fixed count rather than a fixed duration, so every target does the same
 * work and the numbers compare directly - a slow cross-compiled target simply
 * takes longer instead of reporting a different sample.
 *
 * It draws through the memory backend, so it needs no display and runs
 * wherever the tests do. Geometry is generated, which keeps the triangle
 * count an input to the measurement rather than a property of an asset file.
 *
 * The Mpx/s column is frame area per second, not pixels shaded: the quad
 * covers the frame so for it the two coincide, while the sphere leaves most
 * of the frame to the clear, and only its fill rate is directly comparable.
 *
 * Repetition and statistics live here rather than in the calling script, so
 * that what is being timed is compiled code and nothing else. A shell or
 * interpreter loop around the binary measures its own startup and its own
 * jitter alongside the renderer, which on a quiet machine is the larger of
 * the two. --repeat re-runs only the timed loop, inside one process, with
 * the scene already built.
 *
 * The headline figure is the *minimum* over the repeats, not the mean.
 * Interference can only ever make a run slower, so the fastest observed run
 * is the closest thing to the machine's actual capability; the mean drags in
 * every unrelated thing the OS did during the sample. The spread is reported
 * next to it, because a wide spread is what says the minimum is not to be
 * trusted either.
 */

#include "memory_backend.h"

#include "render/transform.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/renderer.h"
#include "render/state.h"
#include "render/texture.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_FRAMES 100
#define MAX_REPEAT 256

// A sphere is a convenient generator: rings x segments x 2 triangles of
// similar size, and it self-occludes, so the depth buffer is exercised too.
#define MAX_RINGS 40
#define MAX_SEGMENTS 40
#define MAX_VERTS ((MAX_RINGS + 1) * (MAX_SEGMENTS + 1))
#define MAX_INDICES (MAX_RINGS * MAX_SEGMENTS * 6)

static Vec3f positions[MAX_VERTS];
static Vec2f coords[MAX_VERTS];
static uint16_t indices[MAX_INDICES];
static Pixel checker[4];

// CLOCK_MONOTONIC would be better but is not everywhere; clock() measures CPU
// time, which is what we want anyway since nothing here waits on anything.
static double elapsed_seconds(clock_t from, clock_t to) {
  return (double)(to - from) / (double)CLOCKS_PER_SEC;
}

// The timings of one case's repeats, reduced. Kept as a struct so the caller
// chooses how to present them and the measuring code has no opinion on it.
typedef struct {
  double best;   // fastest repeat, in seconds - the headline
  double worst;
  double mean;
  double spread; // (worst - best) / best, as a percentage
  int triangles;
  int ok;
} Result;

static void summarize(const double *v, int n, Result *r) {
  double best = v[0], worst = v[0], sum = 0.0;
  for (int i = 0; i < n; i++) {
    if (v[i] < best) best = v[i];
    if (v[i] > worst) worst = v[i];
    sum += v[i];
  }
  r->best = best;
  r->worst = worst;
  r->mean = sum / (double)n;
  r->spread = best > 0.0 ? (worst - best) / best * 100.0 : 0.0;
}

// Builds a unit sphere and returns its triangle count.
static int build_sphere(Mesh *mesh, int rings, int segments) {
  int v = 0;
  for (int r = 0; r <= rings; r++) {
    const float phi = 3.14159265f * (float)r / (float)rings;
    const float sp = sinf(phi), cp = cosf(phi);
    for (int s = 0; s <= segments; s++) {
      const float theta = 6.28318531f * (float)s / (float)segments;
      positions[v] = (Vec3f){sp * cosf(theta), cp, sp * sinf(theta)};
      coords[v] = (Vec2f){(float)s / (float)segments, (float)r / (float)rings};
      v++;
    }
  }

  int n = 0;
  for (int r = 0; r < rings; r++) {
    for (int s = 0; s < segments; s++) {
      const uint16_t a = (uint16_t)(r * (segments + 1) + s);
      const uint16_t b = (uint16_t)(a + segments + 1);
      indices[n++] = a;
      indices[n++] = b;
      indices[n++] = (uint16_t)(a + 1);
      indices[n++] = b;
      indices[n++] = (uint16_t)(b + 1);
      indices[n++] = (uint16_t)(a + 1);
    }
  }

  mesh->index_count = n;
  mesh->pos_indices = indices;
  mesh->tex_indices = indices;
  mesh->positions = positions;
  mesh->tex_coords = coords;
  return n / 3;
}

// Two triangles covering the whole frame: a fill-rate figure rather than a
// geometry one, since the per-triangle setup is negligible here.
static const Vec3f quad_pos[4] = {{-6, -6, 0}, {6, -6, 0}, {6, 6, 0}, {-6, 6, 0}};
static const Vec2f quad_uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
static const uint16_t quad_idx[6] = {0, 1, 2, 0, 2, 3};

static int build_quad(Mesh *mesh) {
  mesh->index_count = 6;
  mesh->pos_indices = quad_idx;
  mesh->tex_indices = quad_idx;
  mesh->positions = quad_pos;
  mesh->tex_coords = quad_uv;
  return 2;
}

// rings == 0 selects the quad; otherwise a sphere of that resolution.
// Measures only; presentation is the caller's business.
static Result measure(int frames, int repeat, int width, int height, int rings,
                      int segments, float distance) {
  Result res = {0};
  double samples[MAX_REPEAT];
  if (repeat > MAX_REPEAT)
    repeat = MAX_REPEAT;

  MemoryBackend backend;
  if (memory_backend_init(&backend, (Vec2i){width, height}) != 0) {
    fprintf(stderr, "  cannot allocate the %dx%d buffers\n", width, height);
    return res;
  }

  Renderer renderer;
  if (renderer_init(&renderer, (Vec2i){width, height},
                    (Backend *)&backend) != OK) {
    fprintf(stderr, "  renderer_init failed\n");
    memory_backend_free(&backend);
    return res;
  }

  Texture texture;
  Material material;
  texture_init(&texture, (Vec2i){2, 2}, checker);
  material_init(&material, &texture);

  Mesh mesh;
  const int triangles =
      rings ? build_sphere(&mesh, rings, segments) : build_quad(&mesh);

  Object object;
  Transform root;
  object_init(&object, &mesh, &material);
  transform_init(&root, (Renderable *)&object, mat4Identity());
  renderer_set_root_renderable(&renderer, (Renderable *)&root);

  renderer.camera.projection =
      mat4Perspective(1, 200.0, (float)width / (float)height, 0.6);
  renderer.camera.view = mat4Translate((Vec3f){0, 0, 0});

  // A sphere's silhouette does not change as it turns, so spinning it varies
  // the triangle slopes without changing the pixel count. A flat quad is the
  // opposite: rotating it takes it edge-on and then behind its own cull, so
  // the cost per frame would fall away as the run got longer. Hence the quad
  // is held still - it is a fill-rate case, and its orientation is the point.
  const int spin = (rings != 0);
  Mat4 translation = mat4Translate((Vec3f){0, 0, -distance});

  // Untimed warmup: it faults in the buffers and lets the clock leave its
  // idle state, both of which otherwise land entirely on the first frames and
  // made short runs look several times slower per frame than long ones.
  const int warmup = frames < 10 ? frames : frames / 10 + 1;
  for (int i = 0; i < warmup; i++) {
    Mat4 rotation = mat4RotateY(spin ? 0.01f * (float)i : 0.0f);
    root.local = mat4MultiplyM(&rotation, &translation);
    if (renderer_render(&renderer) != OK) {
      fprintf(stderr, "  renderer_render failed\n");
      memory_backend_free(&backend);
      return res;
    }
  }

  // Only the inner loop is timed, and the scene is built once outside it, so
  // repeating costs nothing but the frames themselves.
  for (int rep = 0; rep < repeat; rep++) {
    const clock_t start = clock();
    for (int i = 0; i < frames; i++) {
      Mat4 rotation = mat4RotateY(spin ? 0.01f * (float)i : 0.0f);
      root.local = mat4MultiplyM(&rotation, &translation);
      if (renderer_render(&renderer) != OK) {
        fprintf(stderr, "  renderer_render failed\n");
        memory_backend_free(&backend);
        return res;
      }
    }
    samples[rep] = elapsed_seconds(start, clock());
  }

  summarize(samples, repeat, &res);
  res.triangles = triangles;
  res.ok = 1;
  memory_backend_free(&backend);
  return res;
}

// A case is data now, so the scene list is written once and every mode -
// table, TSV, stability - walks the same array instead of restating it.
typedef struct {
  const char *group;
  const char *name;
  int width, height;
  int rings, segments;
  float distance;
  int headline; // reported by --summary, and by the cross-architecture table
} Case;

static const Case CASES[] = {
    {"fill", "quad", 64, 48, 0, 0, 5.0f, 0},
    {"fill", "quad", 160, 120, 0, 0, 5.0f, 0},
    {"fill", "quad", 320, 240, 0, 0, 5.0f, 1},
    {"fill", "quad", 640, 480, 0, 0, 5.0f, 0},
    {"geometry", "sphere 8x8", 320, 240, 8, 8, 3.0f, 0},
    {"geometry", "sphere 20x20", 320, 240, 20, 20, 3.0f, 0},
    {"geometry", "sphere 40x40", 320, 240, 40, 40, 3.0f, 1},
    {"resolution", "sphere 20x20", 64, 48, 20, 20, 3.0f, 0},
    {"resolution", "sphere 20x20", 160, 120, 20, 20, 3.0f, 0},
    {"resolution", "sphere 20x20", 320, 240, 20, 20, 3.0f, 0},
    {"resolution", "sphere 20x20", 640, 480, 20, 20, 3.0f, 0},
};
static const int CASE_COUNT = (int)(sizeof(CASES) / sizeof(CASES[0]));

static void print_row(const Case *c, const Result *r, int frames, int repeat) {
  const double per_frame = r->best / frames;
  printf("  %-14s %4dx%-4d %6d tri  %8.3f s  %8.3f ms/frame  %8.1f fps"
         "  %6.2f Mtri/s  %6.1f Mpx/s",
         c->name, c->width, c->height, r->triangles, r->best,
         per_frame * 1000.0, 1.0 / per_frame, r->triangles / per_frame / 1e6,
         (double)c->width * c->height / per_frame / 1e6);
  if (repeat > 1)
    printf("  +%.1f%%", r->spread);
  printf("\n");
}

// Machine-readable, for the scripts. One row per case, tab separated, with a
// header naming the columns - so a caller selects a field by name instead of
// counting words out of a table meant for a person to read.
static void print_tsv_header(void) {
  printf("group\tscene\twidth\theight\ttriangles\tframes\trepeat"
         "\tbest_s\tworst_s\tmean_s\tspread_pct\tms_per_frame\n");
}

static void print_tsv_row(const Case *c, const Result *r, int frames,
                          int repeat) {
  printf("%s\t%s\t%d\t%d\t%d\t%d\t%d\t%.6f\t%.6f\t%.6f\t%.2f\t%.4f\n", c->group,
         c->name, c->width, c->height, r->triangles, frames, repeat, r->best,
         r->worst, r->mean, r->spread, r->best / frames * 1000.0);
}

// Replaces what used to be an interpreted loop in bench-cpu.sh: the same
// renderer work the real benchmark does, repeated, reporting how much the
// timings move. An interpreted loop measured its own runtime's jitter as much
// as the machine's, which is precisely the thing being checked for here.
static int stability(int frames, int repeat) {
  // The mid-size fill case: long enough that timer granularity does not show,
  // short enough that a repeat is quick, and it touches the whole pipeline.
  const Case *c = &CASES[2];
  printf("stability: %s %dx%d, %d frames x %d repeats\n", c->name, c->width,
         c->height, frames, repeat);

  Result r = measure(frames, repeat, c->width, c->height, c->rings,
                     c->segments, c->distance);
  if (!r.ok) {
    fprintf(stderr, "  measurement failed\n");
    return 2;
  }

  printf("  best %.4f s   worst %.4f s   mean %.4f s\n", r.best, r.worst,
         r.mean);
  printf("  spread %.1f%%\n", r.spread);

  // Two percent is about where a real change stops being distinguishable from
  // the noise on this machine; below it, an A/B of a few percent means
  // something.
  if (r.spread < 2.0) {
    printf("  -> stable enough to A/B\n");
    return 0;
  }
  printf("  -> TOO NOISY: the core is still shared, throttling, or unlocked\n");
  return 1;
}

// Writes just the headline cases' ms/frame to a file, one line, in the order
// they appear above: "18.0385 13.9008".
//
// This exists so the cross-architecture script does no filtering at all. It
// used to read the TSV and pick rows out by scene name and width, which put
// the choice of which cases matter in the shell, in a second place, able to
// drift from this table without anything failing - it would simply report a
// blank column. A file rather than stdout because the build system's own
// progress output shares stdout with the benchmark, and separating them again
// is exactly the filtering being removed.
//
// Only the headline cases are run. Under emulation the other nine cost
// minutes per architecture and were being discarded.
static int summary(int frames, int repeat, const char *path) {
  FILE *f = fopen(path, "w");
  if (!f) {
    fprintf(stderr, "cannot write %s\n", path);
    return 2;
  }

  int bad = 0;
  for (int i = 0; i < CASE_COUNT; i++) {
    const Case *c = &CASES[i];
    if (!c->headline)
      continue;

    Result r = measure(frames, repeat, c->width, c->height, c->rings,
                       c->segments, c->distance);
    if (!r.ok) {
      bad = 1;
      break;
    }
    fprintf(f, "%.4f ", r.best / frames * 1000.0);
    printf("  %-14s %4dx%-4d %8.4f ms/frame  +%.1f%%\n", c->name, c->width,
           c->height, r.best / frames * 1000.0, r.spread);
  }
  fprintf(f, "\n");
  fclose(f);

  if (bad)
    remove(path); // a partial line would be read as a good one
  return bad;
}

static int usage(const char *argv0) {
  fprintf(stderr,
          "usage: %s [frames] [--repeat N] [--tsv|--summary FILE|--stability]\n"
          "  frames         frames per timed run (default %d)\n"
          "  --repeat N     time the run N times, report the best and the spread\n"
          "  --tsv          every case, machine-readable, on stdout\n"
          "  --summary FILE headline cases only; their ms/frame written to FILE\n"
          "  --stability    check how repeatable this machine is right now\n",
          argv0, DEFAULT_FRAMES);
  return 2;
}

int main(int argc, char **argv) {
  int frames = DEFAULT_FRAMES;
  int repeat = 1;
  int tsv = 0;
  int check = 0;
  const char *summary_path = NULL;

  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    if (strcmp(a, "--tsv") == 0) {
      tsv = 1;
    } else if (strcmp(a, "--stability") == 0) {
      check = 1;
    } else if (strcmp(a, "--summary") == 0) {
      if (++i >= argc)
        return usage(argv[0]);
      summary_path = argv[i];
    } else if (strcmp(a, "--repeat") == 0) {
      if (++i >= argc)
        return usage(argv[0]);
      repeat = atoi(argv[i]);
      if (repeat < 1)
        return usage(argv[0]);
    } else if (a[0] == '-') {
      return usage(argv[0]);
    } else {
      // The bare positional stays: callers already pass a frame count.
      frames = atoi(a);
      if (frames < 1)
        return usage(argv[0]);
    }
  }

  checker[0] = pixel_from_rgba(255, 255, 255, 255);
  checker[1] = pixel_from_rgba(60, 60, 60, 255);
  checker[2] = pixel_from_rgba(60, 60, 60, 255);
  checker[3] = pixel_from_rgba(255, 255, 255, 255);

  if (check)
    return stability(frames, repeat > 1 ? repeat : 12);

  if (summary_path)
    return summary(frames, repeat, summary_path);

  if (tsv)
    print_tsv_header();
  else {
    printf("=== Pingo render benchmark: %d frames per case", frames);
    if (repeat > 1)
      printf(", best of %d", repeat);
    printf(" ===\n");
    printf("  %-14s %-9s %10s %8s %14s %10s %13s %11s\n", "scene", "size",
           "triangles", "total", "per frame", "rate", "triangles",
           "frame area");
  }

  int bad = 0;
  const char *group = "";

  for (int i = 0; i < CASE_COUNT; i++) {
    const Case *c = &CASES[i];

    if (!tsv && strcmp(group, c->group) != 0) {
      group = c->group;
      if (strcmp(group, "fill") == 0)
        printf("\nfill rate - two triangles covering the frame\n");
      else if (strcmp(group, "geometry") == 0)
        printf("\ngeometry - a self-occluding sphere at a fixed resolution\n");
      else
        printf("\nresolution - the same 800 triangles at four sizes\n");
    }

    Result r = measure(frames, repeat, c->width, c->height, c->rings,
                       c->segments, c->distance);
    if (!r.ok) {
      bad = 1;
      continue;
    }
    if (tsv)
      print_tsv_row(c, &r, frames, repeat);
    else
      print_row(c, &r, frames, repeat);
  }

  return bad ? 1 : 0;
}
