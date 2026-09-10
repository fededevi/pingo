#!/usr/bin/env bash
#
# Runs every arch-* workflow preset and collects the results into one table.
#
# The presets do the work - this only iterates them, because `cmake --workflow`
# takes a single preset and has no way to skip one whose compiler is absent
# (preset conditions match strings, they cannot probe the filesystem).
#
#   ./scripts/bench-all-arch.sh              every arch preset
#   ./scripts/bench-all-arch.sh aarch64 mips just those, matched by name
#
# FRAMES sets the benchmark length. It is low by default because every target
# but the host runs under emulation. REPEAT re-runs each timed loop inside the
# benchmark process and keeps the best; the repetition is in the compiled
# binary rather than in a loop out here, so what gets timed is the renderer
# and not this script.
#
# TESTS=1 runs each preset's test suite as well. Off by default: this script
# exists to produce timings, and building and running the whole suite under
# qemu costs minutes per architecture for a result `ctest` already gives. It
# also leaves the package thermally saturated right before the timed run,
# which is the opposite of what a stable measurement wants. Use it when the
# question is whether an architecture still works, not how fast it is.
#
set -uo pipefail
cd "$(dirname "$0")/.."

FRAMES=${FRAMES:-20}
REPEAT=${REPEAT:-3}
TESTS=${TESTS:-0}
FILTERS=("$@")

# Every timed run goes on the core bench-cpu.sh reserved, so a number taken
# today is comparable with one taken last week. Without that the host row is
# whichever of this machine's two microarchitectures the scheduler happened to
# pick - the P-cores and the E-cores are not the same CPU, and the gap between
# them dwarfs anything a source change does.
BENCH_CPU=${PINGO_BENCH_CPU:-2}
BENCH_PIN_CPU=""
if [ -e /var/tmp/pingo-bench-cpu.state ]; then
  BENCH_PIN_CPU=$BENCH_CPU
  echo "timing on reserved cpu $BENCH_CPU"
else
  echo "NOTE: no core is reserved - run 'sudo ./scripts/bench-cpu.sh lock' first."
  echo "      Numbers from this run are not comparable with any other session."
fi
echo

presets=$(cmake --list-presets=workflow | grep -oE '"arch-[a-z0-9-]+"' | tr -d '"')
if [ ${#FILTERS[@]} -gt 0 ]; then
  keep=""
  for p in $presets; do
    for f in "${FILTERS[@]}"; do [[ "$p" == *"$f"* ]] && keep="$keep $p"; done
  done
  presets=$keep
fi
# The host is the reference every other row is compared against.
presets="default $presets"

declare -i ok=0 bad=0
declare -a ROWS=()
declare -A MS=()

printf '%-22s %-7s %s\n' PRESET RESULT DETAIL
printf '%s\n' "----------------------------------------------------------------------"

for p in $presets; do
  name=${p#arch-}
  [ "$p" = default ] && name="x86_64 (host)"

  log="build/$p-bench.log"
  mkdir -p build

  fail() {   # $1 = detail shown in the table
    echo "$out" > "$log"
    printf '%-22s %-7s %s\n' "$name" "-" "$1"
    bad+=1
  }

  # Tell a toolchain that is not installed apart from one that is broken.
  missing() {
    echo "$out" | grep -qiE "is not able to compile|CMAKE_C_COMPILER.*not found|No such file or directory.*gcc"
  }

  # Can a binary built for this target actually start here?
  #
  # When it cannot, every test fails identically and instantly, which reads as
  # a defect in the code under test while nothing of ours has run at all. The
  # live example is sparc64 on this distribution: its cross toolchain links
  # _start's GOT computation wrongly - it stores the absolute .got address
  # where the PC-relative delta belongs, so %l7 is off by a constant and the
  # address __libc_start_main computes for main inherits the same error. The
  # process dies before main, and all 22 tests "fail" without a line of pingo
  # executing. qemu closed this as a cross-build bug, not an emulator one
  # (qemu-project/qemu#1807); Debian's toolchain packages are reported to
  # produce working binaries where Ubuntu's do not.
  #
  # A probe rather than a list of known-bad targets: it costs one compile, it
  # names no cause it cannot verify, and it starts passing by itself once the
  # toolchain is fixed - where a list would quietly keep excluding a target
  # that had begun to work.
  probe_runs() {
    local cc rc probe=build/$p/exec-probe
    cc=$(sed -n 's/^CMAKE_C_COMPILER:[A-Z]*=//p' "build/$p/CMakeCache.txt" 2>/dev/null)
    [ -x "$cc" ] || return 0        # cannot tell; assume it runs
    printf 'int main(void){return 0;}\n' > "build/$p/exec-probe.c"
    # Statically linked on purpose: a dynamic probe needs QEMU_LD_PREFIX and
    # exits 255 without it, which is a missing sysroot and not an emulator
    # that cannot run the target - conflating the two marked every working
    # cross target as broken.
    "$cc" -O0 -static -o "$probe" "build/$p/exec-probe.c" >/dev/null 2>&1 || return 0
    # Run it from a separate shell whose stderr is discarded. The probe is
    # expected to die by signal on a target the emulator cannot start, and it
    # is the *waiting* shell that announces "Segmentation fault" - so the
    # redirect has to be on that shell, not on the probe or a subshell.
    # No exec: sh must stay alive as the waiting parent, because it is the
    # waiter that announces "Segmentation fault". With exec, bash inherits
    # the wait and prints it to a stderr this redirect does not cover.
    sh -c '"$0" >/dev/null 2>&1' "$probe" 2>/dev/null
    rc=$?
    [ "$rc" -eq 0 ]
  }

  if ! probe_runs; then
    printf '%-22s %-7s %s\n' "$name" "~" "builds, but its binaries do not start here (toolchain or emulator)"
    ok+=1
    continue
  fi

  if [ "$TESTS" = 1 ]; then
    # The full workflow: configure, build everything, run the suite.
    if ! out=$(cmake --workflow --preset "$p" 2>&1); then
      if missing; then fail "toolchain missing or cannot link"
      elif echo "$out" | grep -q "tests failed"; then
        fail "$(echo "$out" | grep -oE '[0-9]+ tests failed' | head -1) (see $log)"
      else fail "build failed (see $log)"; fi
      continue
    fi
    if echo "$out" | grep -q '100% tests passed'; then
      n=$(echo "$out" | grep -oE 'out of [0-9]+' | tail -1 | grep -oE '[0-9]+')
      detail="$n/$n tests passed"
    else
      detail="built, nothing to run (freestanding)"
    fi
  else
    # Timings only. Building the one target pulls in just the renderer and the
    # maths, so this skips the tests, the examples and the asset suites - most
    # of the work, and all of it irrelevant to a measurement.
    if ! out=$(cmake --preset "$p" \
                 -DPINGO_BENCH_FRAMES="$FRAMES" \
                 -DPINGO_BENCH_REPEAT="$REPEAT" \
                 -DPINGO_BENCH_CPU="$BENCH_PIN_CPU" 2>&1); then
      if missing; then fail "toolchain missing or cannot link"
      else fail "configure failed (see $log)"; fi
      continue
    fi
    # A freestanding preset declares the benchmark target but cannot link it:
    # there is no OS under it, so the binary has no stdout to print to. Asking
    # for it would report a build failure for a target that was never meant to
    # exist there, so build what such a preset does have and say so. Read from
    # the cache rather than matching preset names, which is what the preset
    # itself sets and cannot drift from it.
    if grep -q '^CMAKE_SYSTEM_NAME:.*=Generic$' "build/$p/CMakeCache.txt" 2>/dev/null; then
      if out=$(cmake --build "build/$p" --target pingo_libraries -j"$(nproc)" 2>&1); then
        printf '%-22s %-7s %s\n' "$name" "+" "libraries built, nothing to time (freestanding)"
        ok+=1
      else
        fail "library build failed (see $log)"
      fi
      continue
    fi

    if ! out=$(cmake --build "build/$p" --target render_benchmarks -j"$(nproc)" 2>&1); then
      fail "benchmark build failed (see $log)"
      continue
    fi
    detail="benchmarks built"
  fi

  bin="build/$p/render_benchmarks"

  # Read two numbers out of a file. That is the whole of it.
  #
  # The benchmark decides which cases are headline, runs only those, times
  # them, reduces the repeats and writes the result; the build target knows
  # how to reach a cross-built binary under qemu. Nothing is parsed here,
  # so nothing here can drift out of step with the benchmark.
  if [ -x "$bin" ]; then
    summary="build/$p/bench-summary.txt"
    rm -f "$summary"
    cmake --build "build/$p" --target bench-summary >>"$log" 2>&1
    if [ -f "$summary" ]; then
      read -r q s < "$summary"
      if [ -n "$q" ]; then
        MS["$name"]="$q $s"
        detail="$detail, ${q} ms/frame"
      fi
    fi
  fi

  printf '%-22s %-7s %s\n' "$name" "+" "$detail"
  ok+=1
done

printf '%s\n' "----------------------------------------------------------------------"
printf '%d +   %d -   (%d total)\n' "$ok" "$bad" $((ok + bad))

if [ ${#MS[@]} -gt 1 ]; then
  echo
  pinned_note="unpinned - not comparable across sessions"
  [ -n "$BENCH_PIN_CPU" ] && pinned_note="cpu $BENCH_CPU, fixed clock"
  echo "Render benchmark, $FRAMES frames per case, best of $REPEAT, ms/frame (lower is better)."
  echo "Host timing: $pinned_note."
  echo "Only the host runs natively; the rest are emulated, so read these as a"
  echo "check that each architecture works, not as hardware performance."
  printf '%-22s %14s %14s %10s\n' TARGET "quad 320x240" "sphere 40x40" "vs host"
  printf '%s\n' "------------------------------------------------------------------"
  host=${MS["x86_64 (host)"]:-}
  hq=$(echo "$host" | cut -d' ' -f1)
  for k in "${!MS[@]}"; do
    read -r q s <<<"${MS[$k]}"
    rel=""
    # Integer arithmetic on the fixed 4-decimal TSV fields: "17.9000" becomes
    # 179000, so the ratio is exact to the printed precision without calling
    # out to a second language just to divide.
    if [ -n "$hq" ]; then
      qi=$(( 10#${q%.*}${q#*.} ))
      hi=$(( 10#${hq%.*}${hq#*.} ))
      [ "$hi" -gt 0 ] && rel="$(( (qi + hi / 2) / hi ))x"
    fi
    printf '%-22s %14s %14s %10s\n' "$k" "$q" "$s" "$rel"
  done | sort -t' ' -k1,1
fi

[ "$bad" -eq 0 ]
