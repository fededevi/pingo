#!/usr/bin/env bash
#
# Builds and runs the test suite for every architecture a cross toolchain is
# installed for, the same way CI does it: the compiler comes from the triple
# and qemu's binfmt handler runs the foreign binaries transparently.
#
# Each target gets its own build directory, so a second run rebuilds only what
# changed. Targets whose toolchain is absent are reported and skipped, never
# failed - the point is to use whatever the machine happens to provide.
#
#   ./scripts/build-all-arch.sh              build and test everything
#   ./scripts/build-all-arch.sh --bench      also benchmark each target
#   ./scripts/build-all-arch.sh aarch64 mips just those, matched by name
#
set -uo pipefail
cd "$(dirname "$0")/.."

# name|triple|extra cflags|mode
# mode is 'run' where qemu can execute the result and 'compile' where the
# target is freestanding and there is nothing to run it on.
TARGETS=(
  "x86_64            |                        |                                                  |run"
  "aarch64           |aarch64-linux-gnu       |                                                  |run"
  "riscv64           |riscv64-linux-gnu       |                                                  |run"
  "armhf             |arm-linux-gnueabihf     |                                                  |run"
  "arm-thumb         |arm-linux-gnueabihf     |-mthumb                                           |run"
  "arm-softfloat     |arm-linux-gnueabi       |                                                  |run"
  "s390x             |s390x-linux-gnu         |                                                  |run"
  "sparc64           |sparc64-linux-gnu       |                                                  |run"
  "mips              |mips-linux-gnu          |                                                  |run"
  "mipsel            |mipsel-linux-gnu        |                                                  |run"
  "mips64el          |mips64el-linux-gnuabi64 |                                                  |run"
  "powerpc           |powerpc-linux-gnu       |                                                  |run"
  "powerpc64le       |powerpc64le-linux-gnu   |                                                  |run"
  "m68k              |m68k-linux-gnu          |                                                  |run"
  "riscv32-baremetal |riscv64-unknown-elf     |-march=rv32imac -mabi=ilp32 --specs=picolibc.specs |compile"
)

BENCH=0
FILTERS=()
for arg in "$@"; do
  case "$arg" in
    --bench) BENCH=1 ;;
    -*) echo "unknown option: $arg" >&2; exit 2 ;;
    *) FILTERS+=("$arg") ;;
  esac
done

# Frames per benchmark case. Small by default because everything except the
# host runs under emulation, which is roughly two orders of magnitude slower.
BENCH_FRAMES=${BENCH_FRAMES:-20}

trim() { echo "$1" | sed 's/^ *//; s/ *$//'; }

printf '%-20s %-9s %-26s %s\n' TARGET RESULT TESTS NOTE
printf '%s\n' "--------------------------------------------------------------------------------"

declare -i failed=0 skipped=0 passed=0
declare -a BENCH_ROWS=()

for entry in "${TARGETS[@]}"; do
  IFS='|' read -r name triple cflags mode <<<"$entry"
  name=$(trim "$name"); triple=$(trim "$triple")
  cflags=$(trim "$cflags"); mode=$(trim "$mode")

  if [ ${#FILTERS[@]} -gt 0 ]; then
    match=0
    for f in "${FILTERS[@]}"; do [[ "$name" == *"$f"* ]] && match=1; done
    [ $match -eq 1 ] || continue
  fi

  cc=gcc
  if [ -n "$triple" ]; then
    cc="$triple-gcc"
    if ! command -v "$cc" >/dev/null 2>&1; then
      printf '%-20s %-9s %-26s %s\n' "$name" "skip" "-" "no $cc"
      skipped+=1
      continue
    fi
    # A cross gcc installs without its link-time libc, because
    # libc6-dev-<arch>-cross is only a Recommends. The compiler then works and
    # the linker does not, which surfaces deep inside CMake's compiler probe as
    # a missing Scrt1.o. Check for it here so the report says what is actually
    # wrong. Freestanding targets bring their own libc and are exempt.
    if [ "$mode" != compile ] && \
       ! echo 'int main(void){return 0;}' | "$cc" -x c - -o /dev/null >/dev/null 2>&1; then
      printf '%-20s %-9s %-26s %s\n' "$name" "skip" "-" "$cc cannot link, need libc6-dev-*-cross"
      skipped+=1
      continue
    fi
  fi

  dir="build/arch/$name"
  args=(-S . -B "$dir" -DCMAKE_BUILD_TYPE=Release)
  if [ "$mode" = compile ]; then
    # No OS to link against: static libraries only, and CMake's compiler probe
    # has to be told not to try producing an executable.
    args+=(-DCMAKE_SYSTEM_NAME=Generic
           -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
           -DBUILD_SHARED_LIBS=OFF)
  fi

  log="$dir/build.log"
  mkdir -p "$dir"
  # The cross libc lives in /usr/<triple>; qemu finds nothing without this.
  export CC="$cc" CFLAGS="$cflags" QEMU_LD_PREFIX=${triple:+/usr/$triple}

  if ! cmake "${args[@]}" >"$log" 2>&1; then
    printf '%-20s %-9s %-26s %s\n' "$name" "CONFIG" "-" "see $log"
    failed+=1; continue
  fi

  target=all
  [ "$mode" = compile ] && target=pingo_libraries
  if ! cmake --build "$dir" --target "$target" -j"$(nproc)" >>"$log" 2>&1; then
    printf '%-20s %-9s %-26s %s\n' "$name" "BUILD" "-" "see $log"
    failed+=1; continue
  fi

  warnings=$(grep -c 'warning:' "$log" || true)
  note=""
  [ "$warnings" -gt 0 ] && note="$warnings warnings"

  if [ "$mode" = compile ]; then
    printf '%-20s %-9s %-26s %s\n' "$name" "built" "not run (freestanding)" "$note"
    passed+=1
    continue
  fi

  out=$(ctest --test-dir "$dir" --output-on-failure 2>&1)
  summary=$(echo "$out" | grep -oE '[0-9]+% tests passed, [0-9]+ tests failed out of [0-9]+' | head -1)
  if echo "$out" | grep -q '100% tests passed'; then
    n=$(echo "$summary" | grep -oE 'out of [0-9]+' | grep -oE '[0-9]+')
    printf '%-20s %-9s %-26s %s\n' "$name" "ok" "$n/$n passed" "$note"
    passed+=1
  else
    printf '%-20s %-9s %-26s %s\n' "$name" "TEST" "${summary:-no test output}" "$note"
    failed+=1
  fi

  if [ $BENCH -eq 1 ] && [ -x "$dir/render_benchmarks" ]; then
    row=$("$dir/render_benchmarks" "$BENCH_FRAMES" 2>/dev/null | awk -v n="$name" '
      /^  quad .*320x240/ { for(i=1;i<=NF;i++) if($i=="s"){q=$(i-1);break} }
      /^  sphere 40x40/   { for(i=1;i<=NF;i++) if($i=="s"){s=$(i-1);break} }
      END { if (q!="") printf "%-20s %12.2f %12.2f", n, q*1000/'"$BENCH_FRAMES"', s*1000/'"$BENCH_FRAMES"' }')
    [ -n "$row" ] && BENCH_ROWS+=("$row")
  fi
done

printf '%s\n' "--------------------------------------------------------------------------------"
printf '%d ok, %d failed, %d skipped\n' "$passed" "$failed" "$skipped"

if [ ${#BENCH_ROWS[@]} -gt 0 ]; then
  echo
  echo "Render benchmark, $BENCH_FRAMES frames per case, ms/frame."
  echo "Everything but x86_64 runs under qemu, so compare architectures against"
  echo "each other only with that in mind - these are not hardware numbers."
  printf '%-20s %12s %12s\n' TARGET "quad 320x240" "sphere 40x40"
  printf '%s\n' "--------------------------------------------"
  printf '%s\n' "${BENCH_ROWS[@]}"
fi

[ "$failed" -eq 0 ]
