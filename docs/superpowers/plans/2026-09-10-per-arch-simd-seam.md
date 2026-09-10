# Per-architecture SIMD and assembly seam — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let hand-written SIMD and assembly replace the per-pixel span loop and the 4x4 vertex transform on x86_64, chosen at build time, with the portable C implementation always present and provably equivalent.

**Architecture:** A narrow ABI (`PingoSpan`) hands one fully-covered run of pixels to one function. Every interpolant is linear in x, so the caller passes a start value and a per-pixel delta instead of three edge functions. Implementations live side by side (`span_ref.c`, `span_sse2.S`, `span_avx2.S`); a compile-time inline dispatcher in `span.h` picks one, and all remain callable by name so a differential test can prove them bit-identical.

**Tech Stack:** C99, CMake 3.16+ with presets, GNU as (`.S` via `enable_language(ASM)`), SSE2 and AVX2 intrinsics-free assembly, the existing `render_unit_tests` harness.

**Spec:** `docs/superpowers/specs/2026-09-10-per-arch-simd-seam-design.md`

## Global Constraints

- C standard is C99 (`CMAKE_C_STANDARD 99`, `CMAKE_C_STANDARD_REQUIRED ON`). No C11 or GNU extensions in portable code.
- No dynamic allocation anywhere in `pingo_render` or `pingo_math`. Buffers are supplied by the caller.
- `span_ref.c` compiles and is externally callable on **every** target, including those where `pingo_span` resolves to assembly.
- The golden-image render tests (`render.empty`, `render.triangle`, `render.cube`, `render.no-uv`, `render.group`, and the four `assets.*`) must keep passing **without regenerating references**. If a picture moves, the change is wrong.
- Pixel format is `PINGO_PIXEL_BGRA8888`: `struct Pixel { uint8_t b, g, r, a; }`, 4 bytes.
- Depth format is `ZBUFFER32`: `struct PingoDepth { uint32_t d; }`, `PINGO_DEPTH_MAX = UINT32_MAX`.
- Depth quantisation is `(uint32_t)(value * (float)PINGO_DEPTH_MAX)`. A C cast **truncates toward zero**; vector converts must use the truncating form (`cvttps`), never the rounding form (`cvtps`).
- `(float)UINT32_MAX` is `4294967296.0f`. The reference is therefore undefined at `depth == 1.0f`. Differential tests generate depths in `[0, 1)` and never exactly 1.0.
- Depth test draws when `v >= stored` (`depth_test_and_write` returns false only when `v < stored`), and writes `v`. The comparison is **unsigned**.
- The seam handles only fully-covered runs. `object.c`'s narrow-row path (rows under `PINGO_SPAN_CLIP_MIN_WIDTH`, which is 16) keeps its per-pixel `(w0|w1|w2) < 0` test in C.
- Every commit message ends with:
  `Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>`

## File Structure

| File | Responsibility |
|------|----------------|
| `render/simd/span.h` | The `PingoSpan` struct, the three implementation declarations, and the compile-time inline dispatcher. |
| `render/simd/span_ref.c` | Portable C reference. Always compiled. The definition of correct. |
| `render/simd/span_sse2.S` | x86_64 baseline, 4 px/iteration. |
| `render/simd/span_avx2.S` | AVX2 build only, 8 px/iteration. |
| `math/simd/vec_avx2.h` | `mat4MultiplyVec4` via AVX/FMA, `static inline`. |
| `cmake/PingoSimd.cmake` | Target detection, `PINGO_SIMD` option, `enable_language(ASM)`, one status line. |
| `render/tests/unit/test_span.c` | Differential and boundary tests for every compiled implementation. |
| `render/object.c` | Modified: computes span deltas, calls `pingo_span` for covered runs. |

## Two deliberate deviations from the spec

**No `math/simd/vec_ref.h`.** The spec's file table lists one, but the portable
transform already exists as a `static inline` in `math/mat4.h:28`. Moving it to
a new header would be churn with no reader served, so Task 7 branches inside
the function it is already in.

**`scripts/bench-simd.sh`, not a `bench-avx2` CMake target.** The spec asked
for a target. A CMake target lives inside one build tree, and this comparison
needs three of them (reference, SSE2, AVX2) built by different flags and run
interleaved - the same structural reason `bench-all-arch.sh` cannot be a target
either. Task 8 builds the script instead and drives the existing per-tree
`bench-summary` targets from it.

---

### Task 1: The `PingoSpan` ABI and the C reference

**Files:**
- Create: `render/simd/span.h`
- Create: `render/simd/span_ref.c`
- Test: `render/tests/unit/test_span.c`
- Modify: `render/tests/unit/CMakeLists.txt`, `render/tests/unit/test_render_unit.h`, `render/tests/unit/test_unit_runner.c`

**Interfaces:**
- Consumes: nothing.
- Produces: `PingoSpan` struct; `void pingo_span_ref(const PingoSpan *s)`; `static inline void pingo_span(const PingoSpan *s)`; `int test_span(void)`.

- [ ] **Step 1: Write the failing test**

Create `render/tests/unit/test_span.c`:

```c
#include "test_render_unit.h"

#include "render/simd/span.h"
#include "render/depth.h"
#include "render/pixel.h"
#include "render/texture.h"

// A deliberately naive model of one pixel, written straight from the
// expressions object.c used before the span seam existed. The reference
// implementation must agree with this; the assembly must agree with the
// reference. Keeping the model separate from the reference means a mistake
// has to be made twice, in two different styles, to pass unnoticed.
static void model_pixel(const PingoSpan *s, int i, Pixel *dst, PingoDepth *zt) {
  const float depth = s->depth0 + (float)i * s->ddepth;
  const uint32_t v = (uint32_t)(depth * (float)PINGO_DEPTH_MAX);
  if (v < zt[i].d) {
    return;
  }
  zt[i].d = v;

  if (s->tex == 0) {
    dst[i] = s->flat;
    return;
  }
  const float invW = s->invW0 + (float)i * s->dinvW;
  const float w = 1.0f / invW;
  const Vec2f uv = {(s->uNum0 + (float)i * s->duNum) * w,
                    (s->vNum0 + (float)i * s->dvNum) * w};
  const Pixel t = s->pow2 ? texture_read_uv_pow2(s->tex, uv, s->wmask, s->hmask)
                          : texture_read_uv((Texture *)s->tex, uv);
  dst[i] = s->shade ? pixel_mul_table(t, s->shade) : pixel_mul(t, s->factor);
}

#define SPAN_MAX 64

// Runs impl and the model over identical buffers and requires the colour and
// depth results to be bit-identical, not merely close.
static int agrees(const PingoSpan *proto, void (*impl)(const PingoSpan *),
                  const char *what) {
  Pixel c_impl[SPAN_MAX], c_model[SPAN_MAX];
  PingoDepth z_impl[SPAN_MAX], z_model[SPAN_MAX];

  for (int i = 0; i < SPAN_MAX; i++) {
    const Pixel seed = {(uint8_t)(i * 7), (uint8_t)(i * 13), (uint8_t)(i * 29),
                        255};
    c_impl[i] = c_model[i] = seed;
    // A spread of stored depths so some pixels pass the test and some fail.
    z_impl[i].d = z_model[i].d = (uint32_t)i * 40000000u;
  }

  PingoSpan a = *proto;
  a.dst = c_impl;
  a.depth = z_impl;
  impl(&a);

  for (int i = 0; i < proto->count; i++) {
    model_pixel(proto, i, c_model, z_model);
  }

  for (int i = 0; i < proto->count; i++) {
    TEST_ASSERT_EQ_INT(z_model[i].d, z_impl[i].d, what);
    TEST_ASSERT_EQ_INT(c_model[i].b, c_impl[i].b, what);
    TEST_ASSERT_EQ_INT(c_model[i].g, c_impl[i].g, what);
    TEST_ASSERT_EQ_INT(c_model[i].r, c_impl[i].r, what);
    TEST_ASSERT_EQ_INT(c_model[i].a, c_impl[i].a, what);
  }
  // Nothing past count may be touched.
  for (int i = proto->count; i < SPAN_MAX; i++) {
    TEST_ASSERT_EQ_INT((uint32_t)i * 40000000u, z_impl[i].d, "wrote past count");
  }
  return 1;
}

int test_span(void) {
  PingoSpan s;
  memset(&s, 0, sizeof s);
  s.count = 8;
  s.depth0 = 0.25f;
  s.ddepth = 0.001f;
  s.flat = (Pixel){10, 20, 30, 255};
  s.tex = 0;

  TEST_ASSERT(agrees(&s, pingo_span_ref, "flat fill, ref"), "flat ref");

  // Every vector width's remainder tail.
  static const int counts[] = {0, 1, 4, 7, 8, 9, 16, 17, 31, 32, 33};
  for (size_t i = 0; i < sizeof counts / sizeof counts[0]; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_ref, "flat fill tail, ref"), "flat tail");
  }
  return 1;
}
```

Register the suite. In `render/tests/unit/test_render_unit.h`, after `int test_renderer(void);` add:

```c
int test_span(void);
```

In `render/tests/unit/test_unit_runner.c`, add to the `cases` array after the `renderer` entry:

```c
    {"span", test_span},
```

In `render/tests/unit/CMakeLists.txt`, add `test_span.c` to the `add_executable` source list and `span` to the end of `PINGO_RENDER_UNIT_SUITES`.

- [ ] **Step 2: Run the test to verify it fails**

```bash
cmake --build build/default --target render_unit_tests -j"$(nproc)"
```

Expected: FAIL to compile — `render/simd/span.h: No such file or directory`.

- [ ] **Step 3: Write the ABI header**

Create `render/simd/span.h`:

```c
#pragma once

// One run of pixels, handed to one function.
//
// Everything interpolated across a run is linear in x, so the caller passes a
// start value and a per-pixel delta rather than three edge functions and their
// steps. That removes three multiply-accumulates per pixel from the scalar
// path as well, and it is what makes a vector implementation straightforward:
// a delta becomes a splat and an add.
//
// The run is known to be fully covered. object.c calls this only where
// span_clip has already narrowed the row, so no coverage mask is needed and
// the only per-pixel decision is the depth test.

#include "render/fwd.h"
#include "render/pixel.h"

#include <stdint.h>

typedef struct PingoSpan {
  Pixel *dst;        // &color[x0 + y*width]
  PingoDepth *depth; // &zeta[x0 + y*width]
  int32_t count;     // pixels in the run; may legitimately be 0

  float depth0, ddepth; // depth = depth0 + i*ddepth
  float invW0, dinvW;   // 1/w
  float uNum0, duNum;   // u = uNum/invW
  float vNum0, dvNum;

  const Texture *tex;  // NULL selects the flat-fill path
  int32_t wmask, hmask; // meaningful only when pow2 is set
  int32_t pow2;         // selects the masked sampler

  Pixel flat;                   // used when tex == NULL
  const PixelShadeTable *shade; // NULL selects pixel_mul(text, factor)
  float factor;                 // used when shade == NULL
} PingoSpan;

// The reference. Always compiled, on every target, and externally callable
// even where pingo_span resolves to assembly - the differential test needs
// both reachable by name inside one binary.
void pingo_span_ref(const PingoSpan *s);

#if defined(PINGO_SIMD_AVX2)
void pingo_span_avx2(const PingoSpan *s);
#endif
#if defined(PINGO_SIMD_SSE2)
void pingo_span_sse2(const PingoSpan *s);
#endif

// Compile-time dispatch. No probe, no function pointer, no indirect call:
// the build has already decided, so this collapses to a direct call.
static inline void pingo_span(const PingoSpan *s) {
#if defined(PINGO_SIMD_AVX2)
  pingo_span_avx2(s);
#elif defined(PINGO_SIMD_SSE2)
  pingo_span_sse2(s);
#else
  pingo_span_ref(s);
#endif
}
```

- [ ] **Step 4: Write the C reference**

Create `render/simd/span_ref.c`:

```c
#include "render/simd/span.h"

#include "render/depth.h"
#include "render/texture.h"

// The definition of correct. Every other implementation is required to be
// bit-identical to this, and the differential test in
// render/tests/unit/test_span.c is what establishes that.
//
// Written as two separate loops rather than one loop with a branch: the
// texture test is fixed for the whole span, so hoisting it keeps the flat
// path free of code it never executes, which is the same reason object.c
// hoists it out of the row loop.
void pingo_span_ref(const PingoSpan *s) {
  Pixel *const dst = s->dst;
  PingoDepth *const zt = s->depth;
  const int32_t n = s->count;

  if (s->tex == 0) {
    float depth = s->depth0;
    for (int32_t i = 0; i < n; i++, depth += s->ddepth) {
      if (!depth_test_and_write(zt, (int)i, depth)) {
        continue;
      }
      dst[i] = s->flat;
    }
    return;
  }

  float depth = s->depth0;
  float invW = s->invW0;
  float uNum = s->uNum0;
  float vNum = s->vNum0;

  for (int32_t i = 0; i < n; i++, depth += s->ddepth, invW += s->dinvW,
               uNum += s->duNum, vNum += s->dvNum) {
    if (!depth_test_and_write(zt, (int)i, depth)) {
      continue;
    }
    const float w = 1.0f / invW;
    const Vec2f uv = {uNum * w, vNum * w};
    const Pixel t = s->pow2
                        ? texture_read_uv_pow2(s->tex, uv, s->wmask, s->hmask)
                        : texture_read_uv((Texture *)s->tex, uv);
    dst[i] = s->shade ? pixel_mul_table(t, s->shade) : pixel_mul(t, s->factor);
  }
}
```

`render/CMakeLists.txt` already globs `*.c` recursively and excludes only `tests` and `benchmarks`, so `render/simd/span_ref.c` is picked up with no CMake change.

- [ ] **Step 5: Run the test to verify it passes**

```bash
cmake --build build/default --target render_unit_tests -j"$(nproc)"
./build/default/render_unit_tests span
```

Expected: `PASS: span`.

- [ ] **Step 6: Confirm nothing else moved**

```bash
ctest --test-dir build/default
```

Expected: `100% tests passed, 0 tests failed out of 23` (22 before, plus `render.unit.span`).

- [ ] **Step 7: Commit**

```bash
git add render/simd/span.h render/simd/span_ref.c render/tests/unit/test_span.c \
        render/tests/unit/CMakeLists.txt render/tests/unit/test_render_unit.h \
        render/tests/unit/test_unit_runner.c
git commit -m "$(cat <<'MSG'
render: a span ABI, so one call covers a run of pixels

Everything interpolated across a run is linear in x, so the caller can pass a
start and a delta instead of three edge functions re-evaluated per pixel. That
is what a vector implementation needs, and it removes three multiply-
accumulates per pixel from the scalar path too.

The reference is the definition of correct. The test compares it against a
separately written model of a single pixel, so a mistake has to be made twice,
in two different styles, to pass unnoticed.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 2: Route `object.c`'s covered runs through the seam

**Files:**
- Modify: `render/object.c:300-357`
- Test: existing golden-image tests (`ctest -R render\\.`), plus `render.unit.span`

**Interfaces:**
- Consumes: `PingoSpan`, `pingo_span` from Task 1.
- Produces: no new symbols. `object.c` now populates `PingoSpan` and calls `pingo_span` when `clipped` is true.

- [ ] **Step 1: Establish the baseline the change must not move**

```bash
ctest --test-dir build/default -R 'render\.|assets\.'
```

Expected: all pass. Record the count. This is the test for this task: the pictures must not change.

- [ ] **Step 2: Add the include**

In `render/object.c`, alongside the existing render includes, add:

```c
#include "render/simd/span.h"
```

- [ ] **Step 3: Build the span and call it for covered runs**

In `render/object.c`, the row loop currently computes `w0/w1/w2` from `lo` and then walks x. Replace the body **from** `int32_t w0 = w0_row + lo * A12;` **through** the end of the inner `for (int32_t x = ...)` loop with the following. The `clipped == 0` path keeps its existing per-pixel code verbatim; only the covered path is redirected.

```c
      int32_t w0 = w0_row + lo * A12;
      int32_t w1 = w1_row + lo * A20;
      int32_t w2 = w2_row + lo * A01;

      const int32_t y_base = (minX + lo) + y * scrSize.x;

      if (clipped) {
        // A covered run: every pixel passes coverage, so the whole thing goes
        // to one call. Each interpolant is linear in x, so its per-pixel delta
        // is the same expression as its value with w0/w1/w2 replaced by their
        // steps - no extra derivation, and no multiply-accumulate per pixel.
        PingoSpan s;
        s.dst = &r->target.color.pixels[y_base];
        s.depth = &zeta[y_base];
        s.count = hi - lo + 1;

        s.depth0 = -(w0 * a.z + w1 * b.z + w2 * c.z) * areaInverse;
        s.ddepth = -(A12 * a.z + A20 * b.z + A01 * c.z) * areaInverse;

        if (o->material != 0) {
          s.invW0 = w0 * invAw + w1 * invBw + w2 * invCw;
          s.dinvW = A12 * invAw + A20 * invBw + A01 * invCw;
          s.uNum0 = w0 * tca.x + w1 * tcb.x + w2 * tcc.x;
          s.duNum = A12 * tca.x + A20 * tcb.x + A01 * tcc.x;
          s.vNum0 = w0 * tca.y + w1 * tcb.y + w2 * tcc.y;
          s.dvNum = A12 * tca.y + A20 * tcb.y + A01 * tcc.y;
          s.tex = tex;
          s.pow2 = tex_pow2;
          s.wmask = tex_w_mask;
          s.hmask = tex_h_mask;
          s.shade = use_shade_table ? &shade : 0;
          s.factor = diffuseLight;
        } else {
          s.invW0 = s.dinvW = s.uNum0 = s.duNum = s.vNum0 = s.dvNum = 0.0f;
          s.tex = 0;
          s.pow2 = s.wmask = s.hmask = 0;
          s.shade = 0;
          s.factor = 0.0f;
        }
        s.flat = flat_color;

        pingo_span(&s);
        continue;
      }
```

Leave the existing `for (int32_t x = minX + lo; ...)` loop in place directly below, unchanged — it now serves only the narrow-row case, and its `if (!clipped && ...)` coverage test can be simplified to `if ((w0 | w1 | w2) < 0)` since `clipped` is now always 0 here.

- [ ] **Step 4: Run the golden-image tests**

```bash
cmake --build build/default --target render_tests asset_tests -j"$(nproc)"
ctest --test-dir build/default -R 'render\.|assets\.'
```

Expected: all pass, same count as Step 1, **with no reference regeneration**. A failure here means the delta arithmetic disagrees with the per-pixel form — compare `s.depth0 + i*s.ddepth` against `-(w0' * a.z + ...)` for a hand-worked pixel before touching the references.

- [ ] **Step 5: Confirm the whole suite**

```bash
ctest --test-dir build/default
```

Expected: 23/23.

- [ ] **Step 6: Commit**

```bash
git add render/object.c
git commit -m "$(cat <<'MSG'
render: send covered runs through the span seam

A row that span_clip has narrowed is fully covered, so it can go to one call
instead of a per-pixel loop that re-tests coverage it already knows about.

The interpolant deltas are the same expressions as the values with w0/w1/w2
replaced by their per-x steps, so this derives nothing new - it just stops
recomputing three multiply-accumulates for every pixel.

The narrow-row path is untouched and still tests coverage per pixel; below
sixteen pixels that is cheaper than the setup, which is why span_clip declines
to run there in the first place.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 3: The selection plumbing, reference only

**Files:**
- Create: `cmake/PingoSimd.cmake`
- Modify: `CMakeLists.txt`, `render/CMakeLists.txt`
- Test: configure twice and check the reported selection; full `ctest` both ways

**Interfaces:**
- Consumes: nothing from earlier tasks.
- Produces: cache variable `PINGO_SIMD` (`auto`/`off`); compile definitions `PINGO_SIMD_SSE2` / `PINGO_SIMD_AVX2` on `pingo_render`; CMake variables `PINGO_SIMD_TARGET_X86_64` (bool) and `PINGO_SIMD_SELECTED` (string).

- [ ] **Step 1: Write the module**

Create `cmake/PingoSimd.cmake`:

```cmake
# ---------------------------------------------------------------------------
# Which hand-written implementation of the span seam, if any, this build uses.
#
# Selection is compile-time and nothing else: no cpuid, no function pointers,
# no ifunc. The consequence is accepted - a binary runs only on the ISA it was
# built for - and the gain is that the vertex seam inlines, LTO sees through
# it, and no hot path pays for an indirect call.
#
# The C reference is always compiled, so a target with no hand-written path is
# never broken, only unaccelerated.
# ---------------------------------------------------------------------------

set(PINGO_SIMD "auto" CACHE STRING
    "Span implementation: auto (best for the target) or off (C reference)")
set_property(CACHE PINGO_SIMD PROPERTY STRINGS auto off)

# Ask the compiler for its target rather than reading it off its filename: the
# name need not contain the triple, and frequently does not - a versioned
# gcc-11, a plain "gcc", a ccache wrapper, or clang driven by --target= all
# defeat string surgery. -dumpmachine is the compiler's own answer.
execute_process(
    COMMAND "${CMAKE_C_COMPILER}" -dumpmachine
    OUTPUT_VARIABLE _pingo_triple
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _pingo_triple_rc)
if(NOT _pingo_triple_rc EQUAL 0)
    set(_pingo_triple "")
endif()

set(PINGO_SIMD_TARGET_X86_64 OFF)
if(_pingo_triple MATCHES "^(x86_64|amd64)-")
    set(PINGO_SIMD_TARGET_X86_64 ON)
endif()

set(PINGO_SIMD_SELECTED "reference")

if(PINGO_SIMD STREQUAL "auto" AND PINGO_SIMD_TARGET_X86_64)
    # SSE2 needs no probe: it is part of the x86_64 ABI, so the compiler
    # defines __SSE2__ with no flags passed and there is nothing to fall back
    # to. AVX2 is different - it needs -mavx2, which yields a binary that will
    # not start without it, so it is never chosen automatically. The avx2
    # preset asks for it explicitly.
    set(PINGO_SIMD_SELECTED "sse2")
    if(CMAKE_C_FLAGS MATCHES "-mavx2" OR CMAKE_C_FLAGS MATCHES "-march=x86-64-v3")
        set(PINGO_SIMD_SELECTED "avx2")
    endif()
endif()

if(NOT PINGO_SIMD_SELECTED STREQUAL "reference")
    # Only now, so no target without a hand-written path makes CMake go
    # looking for an assembler it will never use.
    enable_language(ASM)
endif()

message(STATUS "pingo: span implementation is ${PINGO_SIMD_SELECTED}"
               " (target ${_pingo_triple})")
```

- [ ] **Step 2: Include it from the top level**

In `CMakeLists.txt`, immediately after the existing `include(PingoOptimizations)` line:

```cmake
# Which hand-written span implementation this build uses. After
# PingoOptimizations because it may enable_language(ASM), which should see the
# compile options that module has already set.
include(PingoSimd)
```

- [ ] **Step 3: Wire the sources and definitions**

In `render/CMakeLists.txt`, the recursive `*.c` glob already collects
`simd/span_ref.c`. After the `add_library(pingo_render ...)` call add:

```cmake
# The hand-written span, when this build selected one. PUBLIC on the compile
# definition because span.h is installed and its inline dispatcher must resolve
# the same way for consumers as it does here - otherwise a caller would inline
# a call to a symbol this library does not export.
if(PINGO_SIMD_SELECTED STREQUAL "sse2")
    target_sources(pingo_render PRIVATE simd/span_sse2.S)
    target_compile_definitions(pingo_render PUBLIC PINGO_SIMD_SSE2=1)
elseif(PINGO_SIMD_SELECTED STREQUAL "avx2")
    target_sources(pingo_render PRIVATE simd/span_avx2.S)
    target_compile_definitions(pingo_render PUBLIC PINGO_SIMD_AVX2=1)
endif()
```

At this point neither `.S` exists, so guard the task: run only with the default `auto` on x86_64 **after** Task 4 creates `span_sse2.S`. To keep this task independently testable, temporarily verify with `PINGO_SIMD=off`.

- [ ] **Step 4: Verify the reference path configures and passes**

```bash
cmake -B build/simdoff -DCMAKE_BUILD_TYPE=Release -DPINGO_SIMD=off 2>&1 | grep 'pingo: span'
```

Expected: `-- pingo: span implementation is reference (target x86_64-linux-gnu)`

```bash
cmake --build build/simdoff -j"$(nproc)" && ctest --test-dir build/simdoff
```

Expected: 23/23.

- [ ] **Step 5: Verify the selection logic reports sse2 on x86_64**

```bash
cmake -B build/simdsel -DCMAKE_BUILD_TYPE=Release 2>&1 | grep 'pingo: span'
```

Expected: `-- pingo: span implementation is sse2 (target x86_64-linux-gnu)`.
The build will fail for want of `span_sse2.S` — that is expected and Task 4 supplies it. Do not add a stub.

- [ ] **Step 6: Commit**

```bash
git add cmake/PingoSimd.cmake CMakeLists.txt render/CMakeLists.txt
git commit -m "$(cat <<'MSG'
build: select the span implementation at configure time

Compile-time only, so the dispatcher collapses to a direct call and LTO can
see through it. SSE2 needs no probe because it is part of the x86_64 ABI;
AVX2 is never chosen automatically because -mavx2 produces a binary that will
not start on a machine without it.

Asks the compiler for its target with -dumpmachine rather than parsing its
filename, which need not contain the triple - a versioned gcc-11, a plain
"gcc", or clang with --target= all defeat string surgery.

enable_language(ASM) only once a hand-written path is actually selected, so no
target without one goes looking for an assembler.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 4: `span_sse2.S`, flat-fill path

**Files:**
- Create: `render/simd/span_sse2.S`
- Modify: `render/tests/unit/test_span.c`

**Interfaces:**
- Consumes: `PingoSpan` layout from Task 1, `PINGO_SIMD_SSE2` from Task 3.
- Produces: `void pingo_span_sse2(const PingoSpan *s)` — flat path complete, textured path delegating to `pingo_span_ref` until Task 5.

**Struct offsets.** The assembly hard-codes offsets into `PingoSpan`. Derive them once and assert them, never assume:

```bash
cat > /tmp/off.c <<'EOF'
#include "render/simd/span.h"
#include <stdio.h>
#define O(f) printf("#define S_%-8s %2zu\n", #f, offsetof(PingoSpan, f))
int main(void){ O(dst);O(depth);O(count);O(depth0);O(ddepth);O(invW0);
  O(dinvW);O(uNum0);O(duNum);O(vNum0);O(dvNum);O(tex);O(wmask);O(hmask);
  O(pow2);O(flat);O(shade);O(factor);
  printf("/* sizeof = %zu */\n", sizeof(PingoSpan)); return 0; }
EOF
gcc -I. -o /tmp/off /tmp/off.c && /tmp/off
```

Paste the output into `span_sse2.S` as its `#define` block, and add `_Static_assert`-style guards in `span_ref.c` so a struct change breaks the build rather than the pixels. C99 has no `_Static_assert`, so use the negative-array-size idiom:

```c
// Layout the assembly hard-codes. A field reordered here without updating
// render/simd/span_sse2.S would corrupt pixels silently; this makes it a
// compile error instead.
typedef char pingo_span_layout_check[
    (offsetof(PingoSpan, dst) == 0 && offsetof(PingoSpan, depth) == 8 &&
     offsetof(PingoSpan, count) == 16) ? 1 : -1];
```

(Substitute the offsets the helper actually printed.)

- [ ] **Step 1: Extend the test to cover the new implementation**

In `render/tests/unit/test_span.c`, add after the existing reference checks in `test_span`:

```c
#if defined(PINGO_SIMD_SSE2)
  s.count = 8;
  s.tex = 0;
  TEST_ASSERT(agrees(&s, pingo_span_sse2, "flat fill, sse2"), "flat sse2");
  for (size_t i = 0; i < sizeof counts / sizeof counts[0]; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_sse2, "flat tail, sse2"), "flat tail sse2");
  }

  // Depth boundaries: the 2^31 convert boundary is at depth 0.5, and the
  // reference is undefined at exactly 1.0, so approach it without reaching it.
  static const float depths[] = {0.0f, 0.4999999f, 0.5f, 0.5000001f, 0.9999999f};
  for (size_t i = 0; i < sizeof depths / sizeof depths[0]; i++) {
    s.count = 16;
    s.depth0 = depths[i];
    s.ddepth = 0.0f;
    TEST_ASSERT(agrees(&s, pingo_span_sse2, "depth boundary, sse2"), "depth sse2");
  }

  // All pass, all fail, and alternating - the last catches a masked store
  // that writes the wrong lanes.
  s.count = 16; s.ddepth = 0.0f;
  s.depth0 = 1.0f - 1e-7f;   // beats every stored value
  TEST_ASSERT(agrees(&s, pingo_span_sse2, "all pass, sse2"), "all pass");
  s.depth0 = 0.0f;           // loses to every stored value but index 0
  TEST_ASSERT(agrees(&s, pingo_span_sse2, "all fail, sse2"), "all fail");
#endif
```

- [ ] **Step 2: Run to verify it fails**

```bash
cmake --build build/default --target render_unit_tests -j"$(nproc)"
```

Expected: link error, `undefined reference to pingo_span_sse2`.

- [ ] **Step 3: Write the assembly**

Create `render/simd/span_sse2.S`. Algorithm, four pixels per iteration:

The three arithmetic facts that make this exact, and which the differential
test exists to police:

1. **Truncate, do not round.** The reference casts, and a C cast truncates
   toward zero. Use `cvttps2dq`, never `cvtps2dq`.
2. **No packed float-to-unsigned exists before AVX-512.** `cvttps2dq` is
   signed, so a depth at or above 2^31 wraps. For non-negative `d`,
   `floor(d - 2^31) == floor(d) - 2^31` exactly because 2^31 is an integer, so
   compute both `cvttps2dq(d)` and `cvttps2dq(d - 2^31) | 0x80000000` and select
   on `d < 2^31`. A single biased convert would be off by one below the
   boundary, because truncation toward zero is not floor for negatives.
3. **The depth compare is unsigned; SSE2 has only `pcmpgtd`.** XOR both
   operands with `0x80000000` and compare signed. Draw when `v >= stored`,
   i.e. `NOT(stored_biased > v_biased)`.

```asm
/*
 * SSE2 span: four pixels per iteration.
 *
 * SSE2 has no per-lane 32-bit masked store - maskmovdqu is byte-granular,
 * non-temporal and slow - so this loads the destination, blends with
 * pand/pandn/por and stores unconditionally. That makes it read the colour and
 * depth it is about to write, which the scalar path does not, and it is why
 * SSE2's ceiling on an already memory-bound fill is lower than its width
 * suggests.
 */
#define S_dst      0
#define S_depth    8
#define S_count   16
#define S_depth0  20
#define S_ddepth  24
#define S_tex     56
#define S_flat    76

    .text
    .globl  pingo_span_sse2
    .type   pingo_span_sse2, @function
pingo_span_sse2:
    movl    S_count(%rdi), %ecx
    testl   %ecx, %ecx
    jle     .Ldone                  /* count 0 is legitimate */

    cmpq    $0, S_tex(%rdi)
    jne     pingo_span_ref          /* textured: Task 5 replaces this */

    movq    S_dst(%rdi),   %r8
    movq    S_depth(%rdi), %r9

    /* depth_i = depth0 + i*ddepth, four lanes at a time */
    movss   S_depth0(%rdi), %xmm0
    shufps  $0, %xmm0, %xmm0        /* splat depth0 */
    movss   S_ddepth(%rdi), %xmm1
    shufps  $0, %xmm1, %xmm1        /* splat ddepth */
    movaps  .Llane_index(%rip), %xmm2   /* {0,1,2,3} */
    mulps   %xmm1, %xmm2
    addps   %xmm2, %xmm0            /* xmm0 = depth for lanes 0..3 */
    mulps   .Lfour(%rip), %xmm1     /* xmm1 = 4*ddepth, the per-iteration step */

    movd    S_flat(%rdi), %xmm7
    pshufd  $0, %xmm7, %xmm7        /* splat the flat colour */

    cmpl    $4, %ecx
    jl      .Ltail

.Lloop4:
    /* --- quantise / compare / blend body, written out below --- */
    addps   %xmm1, %xmm0            /* advance depth by 4*ddepth */
    addq    $16, %r8                /* 4 pixels  */
    addq    $16, %r9                /* 4 depths  */
    subl    $4, %ecx
    cmpl    $4, %ecx
    jge     .Lloop4

.Ltail:
    /* 1..3 pixels left. Re-run the same body against a 4-lane group but
       commit only the low lanes, by ANDing the keep mask with a lane mask
       taken from .Ltail_mask + 16*count. Reusing the body rather than writing
       a scalar tail is what keeps the tail from being a second, differently
       wrong implementation of the quantisation. */
    testl   %ecx, %ecx
    jle     .Ldone
    /* build lane mask, repeat body once, then fall through */

.Ldone:
    ret
    .size   pingo_span_sse2, .-pingo_span_sse2

    .section .rodata.cst16,"aM",@progbits,16
    .align  16
.Llane_index:  .float 0.0, 1.0, 2.0, 3.0
.Lfour:        .float 4.0, 4.0, 4.0, 4.0
.Lscale:       .float 4294967296.0, 4294967296.0, 4294967296.0, 4294967296.0
.Lbias:        .float 2147483648.0, 2147483648.0, 2147483648.0, 2147483648.0
.Lsignbit:     .long  0x80000000, 0x80000000, 0x80000000, 0x80000000
/* Lane masks indexed by remaining count: 0, 1, 2 or 3 active low lanes. */
.Ltail_mask:   .long 0,0,0,0
               .long -1,0,0,0
               .long -1,-1,0,0
               .long -1,-1,-1,0

    .section .note.GNU-stack,"",@progbits   /* non-executable stack */
```

The quantise-compare-blend body, written out:

```asm
    movaps  %xmm0, %xmm3
    mulps   .Lscale(%rip), %xmm3    /* d = depth * 2^32 */
    movaps  %xmm3, %xmm4
    subps   .Lbias(%rip), %xmm4
    cvttps2dq %xmm3, %xmm5          /* low path:  (int)d           */
    cvttps2dq %xmm4, %xmm6          /* high path: (int)(d - 2^31)  */
    por     .Lsignbit(%rip), %xmm6  /*            | 0x80000000     */
    movaps  %xmm3, %xmm2
    cmpltps .Lbias(%rip), %xmm2     /* mask: d < 2^31 */
    pand    %xmm2, %xmm5
    pandn   %xmm6, %xmm2
    por     %xmm2, %xmm5            /* xmm5 = v, the unsigned depth */

    movdqu  (%r9), %xmm4            /* stored depth */
    movdqa  %xmm4, %xmm6
    pxor    .Lsignbit(%rip), %xmm6
    movdqa  %xmm5, %xmm2
    pxor    .Lsignbit(%rip), %xmm2
    pcmpgtd %xmm2, %xmm6            /* stored > v  ->  pixel rejected */
    /* invert: keep = NOT(stored > v) */
    pcmpeqd %xmm3, %xmm3
    pxor    %xmm3, %xmm6            /* xmm6 = keep mask */

    movdqa  %xmm5, %xmm2            /* blend depth */
    pand    %xmm6, %xmm2
    movdqa  %xmm6, %xmm3
    pandn   %xmm4, %xmm3
    por     %xmm3, %xmm2
    movdqu  %xmm2, (%r9)

    movdqu  (%r8), %xmm4            /* blend colour */
    movdqa  %xmm7, %xmm2
    pand    %xmm6, %xmm2
    movdqa  %xmm6, %xmm3
    pandn   %xmm4, %xmm3
    por     %xmm3, %xmm2
    movdqu  %xmm2, (%r8)
```

- [ ] **Step 4: Run the tests**

```bash
cmake --build build/default -j"$(nproc)" && ./build/default/render_unit_tests span
```

Expected: `PASS: span`. If a depth value differs by exactly 1, check `cvttps2dq` versus `cvtps2dq`. If half the lanes are wrong, check the `pandn` operand order — `pandn a, b` computes `(NOT b) AND a`, which reads backwards.

- [ ] **Step 5: Confirm the pictures did not move**

```bash
ctest --test-dir build/default
```

Expected: 23/23, no reference regeneration.

- [ ] **Step 6: Commit**

```bash
git add render/simd/span_sse2.S render/simd/span_ref.c render/tests/unit/test_span.c
git commit -m "$(cat <<'MSG'
render: SSE2 span, flat-fill path

Four pixels per iteration. SSE2 has no per-lane masked store, so this blends
against the loaded destination instead - which means it reads the colour and
depth it is about to write, and caps what the width can buy on a fill that is
already memory-bound.

Two things had to be exact rather than close. A C cast truncates, so the
convert is cvttps2dq and not cvtps2dq. And no packed float-to-unsigned exists
before AVX-512, so values at or above 2^31 are converted through a biased path
and selected on, rather than biased unconditionally - truncation toward zero is
not floor below the boundary, and a single biased convert is off by one there.

The struct offsets the assembly hard-codes are now guarded by a compile-time
layout check, so reordering a field breaks the build instead of the pixels.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 5: `span_sse2.S`, textured path

**Files:**
- Modify: `render/simd/span_sse2.S`, `render/tests/unit/test_span.c`

**Interfaces:**
- Consumes: everything from Task 4.
- Produces: `pingo_span_sse2` handling `tex != NULL`; the `jne pingo_span_ref` delegation is removed.

- [ ] **Step 1: Extend the test to textured spans**

In `render/tests/unit/test_span.c`, inside the `#if defined(PINGO_SIMD_SSE2)` block, add:

```c
  // A 4x4 power-of-two texture, so both samplers are reachable.
  Pixel texels[16];
  for (int i = 0; i < 16; i++) {
    texels[i] = (Pixel){(uint8_t)(i * 16), (uint8_t)(255 - i * 16),
                        (uint8_t)(i * 3), 255};
  }
  Texture tx;
  texture_init(&tx, (Vec2i){4, 4}, texels);

  s.tex = &tx;
  s.pow2 = 1;
  s.wmask = 3;
  s.hmask = 3;
  s.depth0 = 0.7f;
  s.ddepth = 0.0001f;
  s.invW0 = 2.0f;
  s.dinvW = 0.01f;
  s.uNum0 = 0.3f;
  s.duNum = 0.05f;
  s.vNum0 = 0.8f;
  s.dvNum = -0.03f;
  s.factor = 0.6f;

  s.shade = 0;                       // the pixel_mul path
  for (size_t i = 0; i < sizeof counts / sizeof counts[0]; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_sse2, "textured mul, sse2"), "tex mul");
  }

  PixelShadeTable tbl;                // the table path
  pixel_shade_table_init(&tbl, 0.6f);
  s.shade = &tbl;
  for (size_t i = 0; i < sizeof counts / sizeof counts[0]; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_sse2, "textured table, sse2"), "tex tbl");
  }

  s.pow2 = 0;                        // the general sampler
  s.shade = 0;
  s.count = 16;
  TEST_ASSERT(agrees(&s, pingo_span_sse2, "textured non-pow2, sse2"), "tex gen");

  // Negative u/v: the interpolated coordinate goes negative just outside a
  // triangle, and the reference relies on signed wrap before the cast.
  s.pow2 = 1;
  s.uNum0 = -0.9f;
  s.vNum0 = -0.4f;
  s.count = 16;
  TEST_ASSERT(agrees(&s, pingo_span_sse2, "textured negative uv, sse2"), "tex neg");
```

- [ ] **Step 2: Run to verify it fails**

```bash
cmake --build build/default -j"$(nproc)" && ./build/default/render_unit_tests span
```

Expected: FAIL — the delegation to `pingo_span_ref` will actually pass this
test, so **first delete the `jne pingo_span_ref` line** and re-run to get a
genuine failure (a crash or wrong pixels) before implementing.

- [ ] **Step 3: Implement the textured path**

Extend `span_sse2.S`. Per group of four pixels, after the depth mask is in
`%xmm6`:

- `invW`, `uNum`, `vNum` each get the same splat-and-step treatment as `depth`.
- `w = 1/invW` uses `divps`, not `rcpps`: `rcpps` is an approximation and the
  reference divides, so `rcpps` cannot be bit-identical.
- `u = uNum * w`, `v = vNum * w` by `mulps`.
- Texel index, matching `texture_read_uv_pow2` exactly:
  `sx = (int)(u * (wmask+1)) & wmask`, `sy = (int)(v * (hmask+1)) & hmask`,
  `idx = sx + sy*(wmask+1)`. The `(int)` is again a truncating convert, and the
  mask is what makes negatives wrap correctly in two's complement — so the
  convert must be `cvttps2dq` and the AND must follow it, never precede it.
- **The gather is four scalar loads.** SSE2 has no gather. Extract the four
  indices (`movd` plus `pshufd`, or spill to the stack and reload), load four
  `Pixel`s, and reassemble with `punpckldq`/`punpcklqdq`. This is the part
  least likely to beat the scalar loop; measure it in Task 8 before assuming it
  earned its place.
- Shade, for both the `shade` and `factor` paths identically: unpack bytes to
  32-bit lanes (`punpcklbw` then `punpcklwd` against a zero register),
  `cvtdq2ps`, `mulps` by the splatted factor, `cvttps2dq`, then `packssdw` and
  `packuswb` back to bytes.

  **This is why the table needs no vector lookup.** `pixel_shade_table_init`
  fills `t->v[i] = (uint8_t)(i * f)` — the very expression `pixel_mul` uses — so
  computing the multiply reproduces the table exactly, by construction. Load
  `factor` in both cases and ignore `shade` entirely. AVX2 cannot gather bytes,
  so the alternative would have been four scalar lookups per channel.
- The alpha lane must pass through unchanged: `pixel_mul` keeps `p.a`. Mask the
  alpha byte out of the multiply and OR the original back in.

- [ ] **Step 4: Run the tests**

```bash
cmake --build build/default -j"$(nproc)" && ./build/default/render_unit_tests span
ctest --test-dir build/default
```

Expected: `PASS: span`, then 23/23. If only the alpha channel differs, the
alpha passthrough is missing. If only non-pow2 spans differ, the general
sampler was routed through the masked path.

- [ ] **Step 5: Commit**

```bash
git add render/simd/span_sse2.S render/tests/unit/test_span.c
git commit -m "$(cat <<'MSG'
render: SSE2 span, textured path

Four texels per iteration, but the gather is four scalar loads reassembled -
SSE2 has none - so this is the part of the span least certain to have earned
its place, and Task 8's measurement is what decides.

The shade table needs no vector lookup at all. pixel_shade_table_init fills
v[i] with (uint8_t)(i*f), the same expression pixel_mul evaluates, so
computing the multiply in vector form reproduces the table by construction.
That matters because neither SSE2 nor AVX2 can gather bytes.

Divides rather than reciprocal-estimates: rcpps is an approximation and the
reference divides, so rcpps could not be bit-identical.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 6: The `avx2` preset and `span_avx2.S`

**Files:**
- Create: `render/simd/span_avx2.S`
- Modify: `CMakePresets.json`, `render/tests/unit/test_span.c`

**Interfaces:**
- Consumes: `PingoSpan`, the SSE2 implementation as a model.
- Produces: `void pingo_span_avx2(const PingoSpan *s)`; configure preset `avx2`, build preset `avx2`, workflow preset `avx2`.

- [ ] **Step 1: Add the presets**

In `CMakePresets.json`, add to `configurePresets`:

```json
{
  "name": "avx2",
  "inherits": "base",
  "displayName": "Release, AVX2 (not portable)",
  "description": "Eight pixels per span iteration. -mavx2 produces a binary that will not start on a machine without AVX2, which is why it cannot be the default and needs a preset of its own.",
  "cacheVariables": { "CMAKE_C_FLAGS": "-mavx2 -mfma" }
}
```

and matching entries to `buildPresets` (`{"name": "avx2", "configurePreset": "avx2"}`), `testPresets` (`{"name": "avx2", "configurePreset": "avx2", "output": {"outputOnFailure": true}}`) and `workflowPresets` (configure, build, test steps all named `avx2`).

- [ ] **Step 2: Verify the preset selects AVX2**

```bash
cmake --preset avx2 2>&1 | grep 'pingo: span'
```

Expected: `-- pingo: span implementation is avx2 (target x86_64-linux-gnu)`.
Build fails for want of `span_avx2.S`; that is expected.

- [ ] **Step 3: Extend the test**

In `render/tests/unit/test_span.c`, duplicate the whole SSE2 block under
`#if defined(PINGO_SIMD_AVX2)` with `pingo_span_avx2` in place of
`pingo_span_sse2`, and extend `counts` to include the 8-wide tails — 0, 1, 4,
7, 8, 9, 15, 16, 17, 31, 32, 33. Repeat the code rather than factoring it into
a macro; the two blocks are never both compiled, and a shared macro would hide
which width failed.

- [ ] **Step 4: Write the AVX2 assembly**

Create `render/simd/span_avx2.S`, eight pixels per iteration on `%ymm`
registers. It is the SSE2 algorithm at double width with three changes:

- **A real masked store.** `vpmaskmovd` writes only the lanes the mask selects,
  so the load-blend-store of the SSE2 path disappears, and with it the reads of
  colour and depth that the scalar code never made.
- **`vpmaxud` gives the unsigned compare directly.** `v >= stored` is
  `vpmaxud(v, stored) == v`, so the `0x80000000` biasing that SSE2 needs for
  `pcmpgtd` is unnecessary here.
- **`vgatherdps`/`vpgatherdd` can gather the texels.** 32-bit gather exists in
  AVX2 (byte gather does not, which is why the shade still computes rather than
  looks up). Whether the gather beats eight scalar loads is a real question on
  a small texture that is entirely in L1 — implement scalar loads first, get
  the test green, then try the gather and keep it only if Task 8 says it helps.

Keep `.Lscale`/`.Lbias` as 8-wide `.float` rows, add the `.note.GNU-stack`
section, and end the function with `vzeroupper` before `ret` — omitting it
costs a state-transition penalty in any SSE code the caller runs next.

- [ ] **Step 5: Run the AVX2 tests**

```bash
cmake --build build/avx2 -j"$(nproc)" && ./build/avx2/render_unit_tests span
ctest --test-dir build/avx2
```

Expected: `PASS: span`, then 23/23 — the same golden images as every other
build, since bit-identity is the requirement.

- [ ] **Step 6: Confirm the default build still selects SSE2 and passes**

```bash
ctest --test-dir build/default
```

Expected: 23/23.

- [ ] **Step 7: Commit**

```bash
git add CMakePresets.json render/simd/span_avx2.S render/tests/unit/test_span.c
git commit -m "$(cat <<'MSG'
render: AVX2 span, and a preset to build it

Eight pixels per iteration. Three things get better at this width rather than
merely wider: vpmaskmovd is a real masked store, so the load-blend-store goes
away along with the reads of colour and depth the scalar path never made;
vpmaxud gives the unsigned depth compare directly, without SSE2's sign
biasing; and a 32-bit gather exists for the texels.

Its own preset because -mavx2 yields a binary that will not start without
AVX2, so it cannot be the portable default.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 7: The vertex transform seam

**Files:**
- Create: `math/simd/vec_avx2.h`
- Modify: `math/mat4.h:28` only
- Test: `math/tests/test_mat4.c`

`mat4MultiplyVec4` is already `static inline` in `math/mat4.h:28`, not a
function in `mat4.c`. So the seam is a `#if` **inside the existing inline
function** - no macro redefinition, no rename, and `mat4.c` is not touched.

**Interfaces:**
- Consumes: `PINGO_SIMD_AVX2` from Task 3.
- Produces: `Vec4f mat4MultiplyVec4(const Vec4f *v, const Mat4 *m)` unchanged in signature, with an AVX/FMA body when available.

This seam is `static inline` and never assembly: the body is a handful of
instructions called three times per triangle, so an out-of-line call would cost
more than the vectorisation saves.

- [ ] **Step 1: Write the failing test**

In `math/tests/test_mat4.c`, add a test asserting the transform agrees with a
hand-written scalar dot-product model for a non-trivial matrix, to a tolerance
of zero for exactly representable inputs:

```c
// FMA changes the rounding of a*b+c by not rounding the intermediate, so an
// FMA build can differ from a mul-then-add build in the last bit. Inputs are
// therefore chosen to be exactly representable, where both forms agree
// exactly and a real bug still shows.
static int test_mat4_multiply_vec4_matches_scalar(void) {
  const Mat4 m = {{1.0f,  2.0f,  4.0f,  8.0f,
                   16.0f, 32.0f, 64.0f, 128.0f,
                   0.5f,  0.25f, 0.125f, 0.0625f,
                   2.0f,  4.0f,  8.0f,  16.0f}};
  const Vec4f v = {1.5f, 2.25f, 4.125f, 1.0f};

  const float ex = m.elements[0]*v.x + m.elements[1]*v.y +
                   m.elements[2]*v.z + m.elements[3]*v.w;
  const Vec4f got = mat4MultiplyVec4(&v, &m);
  TEST_ASSERT(got.x == ex, "mat4MultiplyVec4 x");
  return 1;
}
```

Extend to y, z and w with rows 1, 2 and 3. Register it in that file's suite
function the same way its neighbours are registered.

- [ ] **Step 2: Run to verify it fails or passes**

```bash
cmake --build build/default --target math_tests -j"$(nproc)" && \
  ./build/default/math_tests mat4
```

If it passes immediately, the scalar implementation is already correct — that
is fine and expected. The test's purpose is to hold the AVX2 body to the same
answer, so keep it and commit it before writing any intrinsics.

- [ ] **Step 3: Write the AVX2 body**

Create `math/simd/vec_avx2.h`:

```c
#pragma once

// AVX/FMA 4x4-by-vec4. Included by mat4.h only when the build selected AVX2.
//
// static inline and not assembly, deliberately: this is three calls per
// triangle with a body of a few instructions, so an out-of-line call would
// cost more than the vectorisation saves. It is also why 256-bit width does
// not help here - a 4x4 by vec4 is naturally 128-bit, and what AVX2 actually
// contributes is FMA.

#include <immintrin.h>

static inline Vec4f pingo_mat4MultiplyVec4_avx2(const Vec4f *v, const Mat4 *m) {
  const __m128 r0 = _mm_loadu_ps(&m->elements[0]);
  const __m128 r1 = _mm_loadu_ps(&m->elements[4]);
  const __m128 r2 = _mm_loadu_ps(&m->elements[8]);
  const __m128 r3 = _mm_loadu_ps(&m->elements[12]);
  const __m128 vv = _mm_loadu_ps(&v->x);

  // Four row dot products, transposed so each lane holds one result.
  __m128 t0 = _mm_mul_ps(r0, vv);
  __m128 t1 = _mm_mul_ps(r1, vv);
  __m128 t2 = _mm_mul_ps(r2, vv);
  __m128 t3 = _mm_mul_ps(r3, vv);
  _MM_TRANSPOSE4_PS(t0, t1, t2, t3);

  __m128 acc = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

  Vec4f out;
  _mm_storeu_ps(&out.x, acc);
  return out;
}
```

Note this uses `mul` and `add` rather than `_mm_fmadd_ps`. FMA does not round
the intermediate product, so an FMA form can differ from the scalar reference
in the last bit — and the golden images must not move. If Task 8 shows the
transform matters enough to want FMA, that is a separate change with its own
tolerance decision.

In `math/mat4.h`, gate the declaration:

```c
#if defined(PINGO_SIMD_AVX2)
#include "math/simd/vec_avx2.h"
#define mat4MultiplyVec4(v, m) pingo_mat4MultiplyVec4_avx2((v), (m))
#endif
```

Place this **after** the existing `mat4MultiplyVec4` declaration, and confirm
the macro does not break `math/mat4.c`'s definition — if it does, rename the
scalar definition to `pingo_mat4MultiplyVec4_ref` and make the non-AVX2 path a
macro too, so both are symmetrical and both remain callable by name.

- [ ] **Step 4: Run the tests in both builds**

```bash
cmake --build build/default -j"$(nproc)" && ctest --test-dir build/default
cmake --build build/avx2    -j"$(nproc)" && ctest --test-dir build/avx2
```

Expected: 23/23 in both, and identical golden images.

- [ ] **Step 5: Commit**

```bash
git add math/simd/vec_avx2.h math/mat4.h math/mat4.c math/tests/test_mat4.c
git commit -m "$(cat <<'MSG'
math: vectorise the 4x4 by vec4 transform

Inline intrinsics rather than assembly, because this is three calls per
triangle with a body of a few instructions - an out-of-line call would cost
more than the vectorisation saves. It is also why the win is modest: a 4x4 by
vec4 is naturally 128-bit, so AVX2's width contributes nothing and only FMA
would.

Uses mul and add rather than fmadd on purpose. FMA leaves the intermediate
product unrounded, so it can differ from the scalar reference in the last bit,
and the golden images are required not to move.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```

---

### Task 8: Measure it

**Files:**
- Modify: `render/benchmarks/CMakeLists.txt`, `scripts/bench-cpu.sh` (documentation only)
- Create: `scripts/bench-simd.sh`

**Interfaces:**
- Consumes: the `bench-summary` target and `PINGO_BENCH_*` cache variables that already exist; `scripts/bench-cpu.sh lock`.
- Produces: `scripts/bench-simd.sh`, printing one table of C reference against SSE2 against AVX2.

- [ ] **Step 1: Write the comparison script**

Create `scripts/bench-simd.sh`. It must:

- refuse to report numbers unless `/var/tmp/pingo-bench-cpu.state` exists,
  printing the `sudo ./scripts/bench-cpu.sh lock` instruction instead — an
  unpinned run on this machine cannot resolve the single-digit percentage this
  change is expected to produce;
- configure three trees: `build/simd-ref` (`-DPINGO_SIMD=off`),
  `build/default` (SSE2) and `build/avx2`;
- build `render_benchmarks` in each;
- run them **interleaved**, one repetition of each per round rather than all of
  one then all of the next, taking each implementation's best — the machine
  drifts enough between sessions that sequential A/B has produced both a false
  gain and a false regression before;
- read each result from that tree's `bench-summary.txt` and print one table of
  ms/frame plus the percentage against the reference column.

- [ ] **Step 2: Lock the core and check it is measurable**

```bash
sudo ./scripts/bench-cpu.sh lock
./scripts/bench-cpu.sh verify
```

Expected: `spread` under 2% and `-> stable enough to A/B`. If not, stop and
fix that first; no number below is meaningful otherwise.

- [ ] **Step 3: Measure**

```bash
./scripts/bench-simd.sh
```

- [ ] **Step 4: Check the result has the expected shape**

The `quad` cases (full-frame fills, wide covered runs) should improve. The
`sphere 20x20` and `sphere 40x40` cases should barely move, because
`object.c` takes the covered-run path only for rows of at least
`PINGO_SPAN_CLIP_MIN_WIDTH` (16) pixels and a densely tessellated mesh is
mostly narrower rows.

**A uniform improvement across all cases is evidence of a bug, not success** —
most likely the narrow-row path was accidentally routed through the seam. Check
that `clipped` still gates the call.

If AVX2 is not measurably ahead of SSE2 on the quad cases, that is a real
finding and not a failure: the fill is memory-bound, and doubling the register
width does not double memory bandwidth. Record it.

- [ ] **Step 5: Write the numbers into the spec and unlock**

Append a "Measured" section to
`docs/superpowers/specs/2026-09-10-per-arch-simd-seam-design.md` with the
table, one line on whether the shape matched the prediction, and the host
details (`i7-1355U`, cpu 2 locked at 2000 MHz).

```bash
sudo ./scripts/bench-cpu.sh unlock
```

- [ ] **Step 6: Commit**

```bash
git add scripts/bench-simd.sh render/benchmarks/CMakeLists.txt \
        docs/superpowers/specs/2026-09-10-per-arch-simd-seam-design.md
git commit -m "$(cat <<'MSG'
bench: compare the C reference, SSE2 and AVX2 spans

Three trees, interleaved run by run on the reserved core, because the
difference being measured is plausibly single-digit percent and this machine
drifts by twenty between sessions - measuring one build then the other has
already produced both a false gain and a false regression here.

Records the measured table in the design document, including the shape check:
the quad fills should improve and the tessellated spheres should not, because
the seam only takes rows of sixteen pixels or more. A uniform gain would mean
the narrow-row path had been routed through it by mistake.

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
MSG
)"
```
