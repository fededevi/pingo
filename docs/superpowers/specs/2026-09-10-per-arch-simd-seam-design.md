# Per-architecture SIMD and assembly seam

Date: 2026-09-10
Status: approved, not yet implemented

## Purpose

Let hand-written SIMD and assembly replace chosen C functions on
architectures that have something better, without any target losing the
portable implementation and without the renderer's callers knowing which one
they got.

Two seams are in scope, and they are deliberately different shapes:

| seam | granularity | authored as | why |
|------|-------------|-------------|-----|
| textured/flat span | one call per run of pixels | hand-written `.S` | the call is amortised over dozens to hundreds of pixels, so an out-of-line assembly routine costs nothing measurable |
| 4x4 by vec4 transform | 3 calls per triangle | `static inline` intrinsics | the body is a handful of instructions; an out-of-line call would cost more than the vectorisation saves |

## Decisions taken

**Selection is compile-time only.** The build picks one implementation per
target. No `cpuid`, no function pointers, no `ifunc`. The consequence is
accepted: a binary runs only on the ISA it was built for, so anything above
the target's baseline needs its own build. The gain is that the vertex seam
inlines and LTO sees through it, and that nothing in a hot path pays for an
indirect call.

**x86_64 gets SSE2 by default.** SSE2 is part of the x86_64 ABI. The compiler
already defines `__SSE2__` with no flags passed, so this needs no capability
probe, no `-m` flag and no fallback branch on that target.

**AVX2 is a separate build.** It requires `-mavx2`, which yields a binary that
will not start on a machine without it, so it cannot be the portable default.
It gets its own configure/build/workflow preset.

**Everything else gets the C reference.** No other ISA has a hand-written path
in this change. Adding one later is a new `.S` plus one line of CMake, with no
change to callers.

## Layout

```
cmake/PingoSimd.cmake      selection, reporting, enable_language(ASM)
math/simd/
  vec_ref.h                portable C, always present
  vec_avx2.h               AVX/FMA intrinsics, static inline
render/simd/
  span.h                   the ABI, and the PingoSpan argument struct
  span_ref.c               portable C reference, compiled on every target
  span_sse2.S              x86_64 baseline, 4 px/iteration
  span_avx2.S              avx2 preset only, 8 px/iteration
```

`span_ref.c` is compiled and externally callable **on every target**, including
those where `pingo_span` resolves to assembly. The differential test needs both
implementations reachable by name inside one binary.

## The span ABI

Everything interpolated across a run is linear in x. Rather than pass the three
edge functions and their per-pixel steps and re-derive each interpolant per
pixel, `object.c` computes a start value and a delta once per span:

```c
typedef struct {
    Pixel      *dst;         /* &color[x0 + y*width]                       */
    PingoDepth *depth;       /* &zeta[x0 + y*width]                        */
    int32_t     count;       /* pixels in the run; may be 0                */

    float depth0, ddepth;    /* depth = depth0 + i*ddepth                  */
    float invW0,  dinvW;     /* 1/w                                        */
    float uNum0,  duNum;     /* u/w numerator; u = uNum/invW               */
    float vNum0,  dvNum;

    const Texture *tex;      /* NULL selects the flat-fill path            */
    int32_t        wmask, hmask;  /* valid only when the texture is pow2   */
    int32_t        pow2;     /* selects the masked sampler                 */

    Pixel                flat;   /* used when tex == NULL                  */
    const PixelShadeTable *shade; /* NULL selects pixel_mul(text, factor)  */
    float                 factor; /* used when shade == NULL               */
} PingoSpan;

void pingo_span(const PingoSpan *s);
```

This refactor is worth doing on its own account: in the C path it replaces
three multiply-accumulates per pixel with one addition each.

`shade` being nullable mirrors the existing `use_shade_table` choice, which
`object.c` makes per triangle from `PINGO_SHADE_TABLE_MIN_AREA` (256). The
span implementation does not re-derive that decision.

## Scope limit: fully-covered runs only

The seam handles only the run that `span_clip` has already narrowed, where
every pixel is known to pass coverage. The narrow-row path keeps its per-pixel
`(w0 | w1 | w2) < 0` test in C.

**This bounds the expected benefit, and the bound is not small.** `object.c`
takes the clipping path only when the row is at least
`PINGO_SPAN_CLIP_MIN_WIDTH` (16) pixels wide; below that its own comment notes
the per-pixel sign test is cheaper and "the common case for a densely
tessellated mesh". So:

- the `quad` benchmark cases (full-frame fills at 64x48 through 640x480) are
  almost entirely wide covered runs and should improve;
- the `sphere` cases at 20x20 and 40x40 are mostly narrow rows and should
  barely move.

Anyone reading a benchmark table for this change should expect exactly that
shape, and a uniform improvement across all cases would be evidence something
is wrong.

Extending the seam to masked coverage is possible later; it is left out because
it forces a coverage mask through the whole vector path to accelerate the
minority of pixels.

## The AVX2 and SSE2 span implementations

8 and 4 pixels per iteration respectively, over packed BGRA8888. Coverage is
guaranteed by the scope limit, so the only per-pixel mask is the depth test.

AVX2 applies it with `vpmaskmovd`. SSE2 has no per-lane 32-bit masked store -
`maskmovdqu` is byte-granular, non-temporal and slow - so the SSE2 path must
load the destination, blend with `pand`/`pandn`/`por`, and store unconditionally.
That makes the SSE2 span read the colour and depth buffers it is about to
write, which the scalar code does not do; it is correct but it caps what SSE2
can gain on a fill that is already memory-bound.

The arithmetic is straightforward. Two details are not, and both are places a
silent mismatch with the C reference would hide:

**Depth quantisation must truncate, not round.** The reference computes
`(uint32_t)(value * (float)UINT32_MAX)`, and a C cast truncates toward zero.
The vector convert must therefore be `cvttps` (truncating), not `cvtps`, which
rounds to nearest under the prevailing rounding mode. Using the wrong one
differs by one least-significant depth bit on about half of all inputs.

**Neither SSE2 nor AVX2 has a packed float-to-unsigned convert.** That arrives
with AVX-512; `cvttps_epi32` is signed, so every depth value at or above 2^31
wraps negative. `object.c` has already rejected anything outside `[0, 1]`
before the span begins, so the full unsigned range is reachable and this is not
a theoretical concern. Approach: bias by 2^31 before the truncating convert and
flip the sign bit afterwards, which reproduces the unsigned result exactly.

**The reference overflows at `depth == 1.0`, and the test must not chase it.**
`PINGO_DEPTH_MAX` is `UINT32_MAX`, and `(float)UINT32_MAX` rounds up to
4294967296.0f, which is not representable in `uint32_t`. So for a depth of
exactly 1.0 - a value `object.c` explicitly admits, since it rejects only
`> 1.0f` - the reference's cast is undefined behaviour. "Bit-identical" is
therefore not a meaningful requirement at that one input.

Decision: the differential test generates depths in `[0, 1)` and excludes
exactly 1.0. The underlying reference defect is recorded as follow-up work
below rather than fixed here, because changing the quantisation would move
every golden image and belongs in its own change.

**The depth comparison is unsigned and keeps the maximum.**
`depth_test_and_write` returns false when `v < stored`, so a pixel is drawn
when `v >= stored`. Neither SSE2 nor AVX2 has an unsigned 32-bit compare, but
AVX2 has `vpmaxud`, so `v >= stored` is `max(v, stored) == v`. SSE2 has no
`pmaxud` either; there the comparison is done by biasing both operands by 2^31
and using the signed `pcmpgtd`.

## Build and bench

New presets, mirroring the existing structure:

- configure preset `avx2`, inheriting `base`, adding `-mavx2 -mfma`
- matching build and workflow presets, so it is tested like any other target
- `bench`, `bench-tsv`, `bench-summary` and `bench-stability` come along
  automatically in `build/avx2`

`-DPINGO_SIMD=off` forces the C reference in any tree. It is needed to measure
what the assembly actually buys — with SSE2 as the default, the interesting
comparison is each implementation against the C baseline, not merely AVX2
against SSE2 — and to bisect a suspected miscompile without editing source.

A `bench-avx2` target builds both trees and runs them **interleaved** on the
reserved core. Measuring one and then the other is the error
`scripts/bench-cpu.sh` and the interleaving note exist to prevent: 8-wide
against 4-wide on a memory-bound fill is plausibly a single-digit percentage,
which is inside this machine's drift band unless the core is locked and the
runs alternate.

## Testing

**Differential, bit-exact.** A new test drives `span_ref` and `pingo_span` over
randomised `PingoSpan` inputs and asserts the colour and depth buffers are
bit-identical afterwards. Golden images cannot do this job: the render tests
tolerate a few differing pixels, which the README already identifies as how
drift hides.

On a target with no hand-written path the test compares the reference against
itself. That is intentional - it keeps the test unconditional, and it still
exercises the `PingoSpan` refactor.

Boundary cases, each asserted explicitly:

- `count` of 0, 1, 4, 7, 8, 9, 16, 17 — the vector remainder tails for both widths
- every pixel passing the depth test, and every pixel failing it
- alternating pass/fail, to catch a masked store that writes the wrong lanes
- depth values at 0.0, either side of 0.5 (the 2^31 convert boundary), and the
  largest representable value below 1.0 - but never exactly 1.0, per the
  overflow decision above
- `tex == NULL` flat fill
- `shade == NULL` (the `pixel_mul` path) and non-NULL (the table path)
- pow2 and non-pow2 samplers

The existing golden-image render tests stay as they are and must not change.
If a picture moves, the change is wrong.

## Out of scope

- Runtime CPU detection or any dispatch that is not compile-time
- NEON, and any ISA other than SSE2 and AVX2
- Vectorising the narrow-row path or texture sampling
- Vertex transform in assembly - it must inline, so it stays intrinsics
- The sparc64 toolchain problem, which is unrelated and documented separately

## Follow-up recorded, not done here

`depth_test_and_write` multiplies by `(float)PINGO_DEPTH_MAX`, which for
ZBUFFER32 rounds to 2^32 and makes the cast undefined at `depth == 1.0`.
Scaling by `PINGO_DEPTH_MAX` as a float one ulp below 2^32, or clamping before
the cast, would fix it. Both change quantisation for every pixel and so move
every golden image, which is why it is not bundled into this change.
