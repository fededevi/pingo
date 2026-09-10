# Per-architecture SIMD seam — measured results

Running record, one row per completed task. Numbers are ms/frame, lower is
better.

**Plan:** `docs/superpowers/plans/2026-09-10-per-arch-simd-seam.md`
**Spec:** `docs/superpowers/specs/2026-09-10-per-arch-simd-seam-design.md`

## Method

Host is an i7-1355U, a hybrid part: CPUs 0-3 are P-cores at up to 5.0 GHz
(SMT pairs 0-1, 2-3), CPUs 4-11 are E-cores at up to 3.7 GHz. An unpinned run
times whichever microarchitecture the scheduler picked, which is worth more
than anything measured here.

Every row below was taken with `sudo ./scripts/bench-cpu.sh lock`: CPU 2
pinned at 2000 MHz min=max, its SMT sibling CPU 3 offlined, `system.slice` and
`init.scope` evicted from the core, IRQs steered away. 2000 MHz rather than the
5.0 GHz turbo because this is a 15 W part - it holds turbo for seconds and then
measures the cooler.

```sh
sudo ./scripts/bench-cpu.sh lock      # then verify: spread must be under 2%
taskset -c 2 <tree>/render_benchmarks 60 --repeat 7 --tsv
```

Repetition and statistics are inside the compiled benchmark, and the figure is
the **minimum** over 7 repeats: interference can only make a run slower, so the
fastest is closest to the machine's capability. The `spread` column is
(worst-best)/best on the quad 320x240 case and says whether to trust the row -
above about 2% the row is noise.

Cases: `quad` is a frame-covering fill (wide covered runs, the case the seam
targets); `sph` is a self-occluding sphere at 20x20 and 40x40 tessellation
(mostly rows under 16 pixels, which the seam deliberately does not touch).

## Results

| state | quad 320 | quad 640 | sph 20 | sph 40 | spread |
|-------|---------:|---------:|-------:|-------:|-------:|
| 0 baseline (master) | 0.8692 | 3.4939 | 0.5621 | 0.7530 | 0.4% |
| 1 span ABI + reference | 0.8720 | 3.4979 | 0.5600 | 0.7516 | 0.7% |
| 2 object.c uses the seam | 0.8400 | 3.3761 | 0.5520 | 0.7450 | 0.6% |
| 3 selection plumbing (ref) | 0.8404 | 3.3861 | 0.5541 | 0.7455 | 0.3% |
| 4 SSE2 flat path | 0.8554 | 3.4360 | 0.5578 | 0.7466 | 0.4% |
| 5 SSE2 textured path | 0.6361 | 2.4883 | 0.5596 | 0.7549 | 0.4% |

## Notes per row

**0 → 1: no change, and that is the correct result.** Nothing calls the seam
yet; `object.c` is untouched. The two rows agreeing inside the 0.7% spread is a
control on the rig, not a result about the code.

It also records a lost expectation. The plan claimed Tasks 1-3 would speed up
the scalar path on their own by replacing three multiply-accumulates per pixel
with one addition. They do not, because that simplification turned out not to
be bit-identical - see the Task 1 commit. The ABI now recomputes each
interpolant per pixel from exact integer barycentrics, which is the same
arithmetic as before. Any speedup must come from the vector implementations.

**1 → 2: -3.4% on quad 320, -3.5% on quad 640, with the arithmetic unchanged.**
Every expression is bit-identical - the golden images did not move and were not
regenerated - so none of this came from doing the maths differently. It came
from work that was never arithmetic:

- the covered path no longer evaluates `!clipped && (w0 | w1 | w2) < 0` once
  per pixel to re-establish coverage `span_clip` already proved;
- the narrow-row condition lost its `clipped` term, so it is one test rather
  than two;
- `o->material != 0` is hoisted out of the pixel loop into the reference's
  two-loop split, instead of being branched on per pixel.

The spheres improved about 1%, which is the expected shape: they are mostly
rows under sixteen pixels, so they never take the covered path and gain only
from the simplified condition.

Worth noting against the plan's prediction: this row was supposed to be flat,
because the multiply-accumulate saving it promised had to be given up for
bit-identity. It is not flat, for reasons that have nothing to do with the
interpolants.

**2 → 3: flat.** Plumbing only; the reference is still selected.

**3 → 4: +1.8% on the fills, and that is not noise - it is a branch.** Every
benchmark case is textured (`benchmark_render.c` always builds a material), so
nothing exercises the flat path at all. The SSE2 routine tests `tex`, finds it
set, and tail-calls the reference on every span; the extra test-and-jump is the
whole effect. Task 4 bought the infrastructure, not the speed.

**4 → 5: -26.8% on quad 320, -28.8% on quad 640; spheres inside noise.** This
is the shape the spec predicted, and it is the check that the seam is wired
correctly: the spheres are mostly rows under sixteen pixels, which never enter
the span, so a uniform improvement would have meant the narrow-row path had
been routed through by mistake. The gain is depth, coverage-free rasterisation
and shading four pixels at a time; the texel fetch is still four scalar loads
because SSE2 has no gather.

## SSE2 against AVX2, interleaved

The rows above were taken minutes apart, and this machine drifts about 2.5%
between them even with the core locked - the SSE2 row re-measured at 0.6517
after reading 0.6361. That is inside a two-build comparison, so the AVX2 number
is taken interleaved: the two binaries alternate run by run for six rounds and
each keeps its best.

| case | SSE2 (default) | AVX2 (avx2 preset) | change |
|------|---------------:|-------------------:|-------:|
| quad 320x240 | 0.6325 | 0.5489 | -13.2% |
| quad 640x480 | 2.4886 | 1.7407 | -30.1% |
| sphere 20x20 | 0.5580 | 0.6802 | **+21.9%** |
| sphere 40x40 | 0.7493 | 0.7344 | -2.0% |

**The fills gain, sphere 20x20 loses, and the loss is real.** Six interleaved
rounds reproduce it. That mesh has 800 large triangles at 320x240, so many rows
do exceed sixteen pixels and enter the span - but as short runs of 16-40
pixels, where AVX2's per-span setup and its `vpgatherdd` are not amortised.
The plan said to try scalar loads first and keep the gather only if measured;
that step was skipped, and the next row is the measurement it should have had.

## Is it the gather? No.

Three trees interleaved for six rounds: SSE2, AVX2 with `vpgatherdd`, and AVX2
with the gather replaced by eight scalar loads through the frame.

| case | SSE2 | AVX2 + gather | AVX2, scalar loads |
|------|-----:|--------------:|-------------------:|
| quad 320x240 | 0.6329 | **0.5490** | 0.6296 |
| quad 640x480 | 2.4870 | **1.7410** | 2.0831 |
| sphere 20x20 | 0.5576 | 0.6792 | 0.6835 |
| sphere 40x40 | 0.7489 | 0.7351 | 0.7338 |

**The hypothesis is falsified.** Scalar loads regress sphere 20x20 by the same
22%, so the gather is not what is slow on short spans. And the gather is a real
win on long ones - 16% faster than scalar loads on the 640x480 fill - so it
stays.

What remains is fixed per-span cost: thirteen splats, three eight-lane vector
builds, an 832-byte frame, and eight-wide iterations that waste up to seven
lanes on a seventeen-pixel run where four-wide wastes none. AVX2 pays for that
only once a span is long enough to amortise it.

**Decision: dispatch by span length, at compile time.** The avx2 build also
carries `span_sse2.S`, and the inline dispatcher in `span.h` sends runs below
`PINGO_AVX2_MIN_SPAN` to it. No function pointer - one compare on a value the
caller already holds. The threshold is swept below rather than guessed.

## The threshold sweep, and a wrong premise corrected

Five trees interleaved for six rounds: SSE2, and the hybrid at
`PINGO_AVX2_MIN_SPAN` of 16, 32, 64 and 128.

| threshold | quad 320 | quad 640 | sphere 20 | sphere 40 |
|-----------|---------:|---------:|----------:|----------:|
| SSE2 only | 0.6331 | 2.4871 | 0.5581 | 0.7480 |
| 16 | 0.5486 | 1.7394 | 0.5602 | 0.7321 |
| **32** | 0.5488 | 1.7418 | **0.5502** | 0.7346 |
| 64 | 0.5395 | 1.7393 | 0.5504 | 0.7342 |
| 128 | 0.5318 | 1.7394 | 0.5510 | 0.7330 |

Every hybrid threshold cures the sphere 20x20 regression. That looked wrong at
first: a threshold of 16 seemed functionally identical to pure AVX2, because
`object.c` only enters the seam when the row is at least sixteen wide. **The
premise was false.** `clipped` is decided on the bounding-box row width, and
`span_clip` then narrows the run - so a triangle tip crossing a wide row yields
a covered run of one to fifteen pixels. Those are the spans where AVX2's fixed
per-span cost dominates, and every threshold from 16 up routes them to SSE2.
Confirmed by building HEAD's pure-AVX2 in a worktree and interleaving it
against the 16-threshold hybrid: 0.6789 against 0.5595 on sphere 20x20, with
the fills identical.

**Threshold: 32.** Sphere 20x20 bottoms out there (0.5502), 16 is measurably
worse (0.5602 - runs of 16-31 still favour SSE2), and above 32 nothing moves
outside noise. The 3% spread on quad 320 between 32 and 128 is with identical
routing - every full-width span goes to AVX2 regardless - so it is code layout
or drift, not dispatch, and is the reason the loop heads are now aligned.

## Where this leaves the numbers

Against the master baseline, on the reserved core:

| case | baseline | SSE2 (default) | AVX2 hybrid (avx2 preset) |
|------|---------:|---------------:|--------------------------:|
| quad 320x240 | 0.8692 | 0.6331 (-27%) | 0.5488 (-37%) |
| quad 640x480 | 3.4939 | 2.4871 (-29%) | 1.7418 (-50%) |
| sphere 20x20 | 0.5621 | 0.5581 (-1%) | 0.5502 (-2%) |
| sphere 40x40 | 0.7530 | 0.7480 (-1%) | 0.7346 (-2%) |

The shape is the one the spec predicted: fills gain, tessellated meshes barely
move because their rows are mostly too narrow to enter the seam. Every row of
every table above was produced by binaries that pass the same 23 tests against
the same unmodified golden images.

## Task 7, the vertex transform: measured and rejected

An AVX version of `mat4MultiplyVec4` was built to the spec's requirement of
bit-identity: four row loads, four multiplies, an eight-shuffle transpose, and
a *sequential* sum so the scalar's `((x+y)+z)+w` order is preserved. The
exact-equality test proved that requirement bites - switching to the pairwise
`(t0+t1)+(t2+t3)` order failed it on the first input.

Interleaved, six rounds, the avx2 tree with and without the vector transform:

| case | vector transform | scalar transform | change |
|------|-----------------:|-----------------:|-------:|
| quad 320x240 | 0.5492 | 0.5496 | 0 |
| quad 640x480 | 1.7447 | 1.7415 | 0 |
| sphere 20x20 | 0.5836 | 0.5513 | **+5.9%** |
| sphere 40x40 | 0.8547 | 0.7344 | **+16.4%** |

**Slower, on exactly the cases that exercise it.** The fills have two triangles;
sphere 40x40 has 3200, about nineteen thousand transforms a frame. The
transpose and the store-and-reload of the returned `Vec4f` cost more than the
scalar code the compiler emits, which stays in registers. The faster
reductions - `dpps`, `hadd` - sum in a different order and would move the
golden images, which is the constraint that rules them out.

Removed. The finding is recorded in a comment on the function so the next
person does not repeat the experiment without first reading why it lost. The
spec's prediction was "a small win"; the measurement says a loss, and the
measurement stands.

This is what bench-simd.sh first showed as a +10.6% regression on sphere
40x40 against the reference: it ran immediately after Task 7 landed and the
delta was the transform, not the span.

## Final, as shipped

`./scripts/bench-simd.sh`, eight interleaved rounds, core 2 locked at 2000 MHz,
after the vertex transform was removed:

| build | quad 320x240 | vs reference | sphere 40x40 | vs reference |
|-------|-------------:|-------------:|-------------:|-------------:|
| C reference (`-DPINGO_SIMD=off`) | 0.8413 | - | 0.7743 | - |
| SSE2 (`default` preset) | 0.6337 | **-24.6%** | 0.7505 | -3.0% |
| AVX2 hybrid (`avx2` preset) | 0.5502 | **-34.6%** | 0.7336 | -5.2% |

Against the master baseline taken at the start (0.8692 / 0.7530), the default
x86_64 build is 27% faster on the fill; the AVX2 build 37%, and 50% at 640x480.
Tessellated meshes move a few percent, by design.

Every binary in every table passes the same 23 tests against the same golden
images, unmodified since before the work began.

What the measurements decided, against what was planned:

- The span ABI carries integer barycentrics, not float start-and-delta pairs -
  the differential test rejected the plan's version on its first run.
- The AVX2 build also carries SSE2 and dispatches on span length; pure AVX2 was
  22% slower on short runs, which clipping produces far more often than the
  gating threshold implies.
- `vpgatherdd` stays; scalar loads were suspected and cleared.
- The vertex transform is not vectorised; it measured slower under the
  bit-identity constraint.
- The avx2 preset disables FP contraction, or the C reference itself would
  render differently from the default build.
