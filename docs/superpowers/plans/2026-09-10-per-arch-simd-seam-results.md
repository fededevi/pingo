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
