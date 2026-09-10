#!/usr/bin/env bash
#
# The C reference against SSE2 against AVX2, interleaved.
#
# Three build trees, run alternately round by round. Measuring one to
# completion and then the next has produced both a false gain and a false
# regression on this machine: it drifts a few percent between minutes even
# with the core locked, and the difference being measured is sometimes
# smaller than that. Alternating puts the drift on every build equally.
#
# The shell does no arithmetic on the timings. The benchmark's --summary keeps
# the smaller of the old and new value per case when the file already exists,
# so after N rounds each tree's file holds its best, and this script only
# reads two numbers out of each.
#
#   sudo ./scripts/bench-cpu.sh lock     first - this refuses to run unpinned
#   ./scripts/bench-simd.sh              6 rounds
#   ROUNDS=12 ./scripts/bench-simd.sh
#
set -uo pipefail
cd "$(dirname "$0")/.."

ROUNDS=${ROUNDS:-6}
FRAMES=${FRAMES:-40}
REPEAT=${REPEAT:-3}
CPU=${PINGO_BENCH_CPU:-2}

if [ ! -e /var/tmp/pingo-bench-cpu.state ]; then
  echo "no core is reserved. Run:  sudo ./scripts/bench-cpu.sh lock" >&2
  echo "An unpinned run cannot resolve the differences this compares." >&2
  exit 1
fi

# name | tree | how to configure it
trees=(
  "reference|build/simd-ref|-DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DPINGO_SIMD=off"
  "sse2     |build/default |--preset default"
  "avx2     |build/avx2    |--preset avx2"
)

for row in "${trees[@]}"; do
  IFS='|' read -r name tree cfg <<<"$row"
  tree=${tree// /}
  # shellcheck disable=SC2086
  if [[ $cfg == --preset* ]]; then cmake $cfg >/dev/null 2>&1
  else cmake -B "$tree" $cfg >/dev/null 2>&1; fi
  cmake --build "$tree" --target render_benchmarks -j"$(nproc)" >/dev/null 2>&1 \
    || { echo "build failed: $tree" >&2; exit 1; }
  rm -f "$tree/bench-simd.txt"
done

echo "interleaving ${#trees[@]} builds, $ROUNDS rounds, $FRAMES frames x $REPEAT repeats, cpu $CPU"
for _ in $(seq "$ROUNDS"); do
  for row in "${trees[@]}"; do
    IFS='|' read -r _ tree _ <<<"$row"; tree=${tree// /}
    taskset -c "$CPU" "$tree/render_benchmarks" "$FRAMES" --repeat "$REPEAT" \
      --summary "$tree/bench-simd.txt" >/dev/null 2>&1
  done
  printf '.'
done
echo

# Percentages against the reference, as integer math on the fixed 4-decimal
# fields - the same trick bench-all-arch.sh uses to avoid a second language.
pct() { # $1 value, $2 base -> "-27.3%"
  local v=$(( 10#${1%.*}${1#*.} )) b=$(( 10#${2%.*}${2#*.} ))
  [ "$b" -gt 0 ] || { echo "  n/a"; return; }
  local d=$(( (v - b) * 1000 / b ))       # tenths of a percent
  printf '%+d.%d%%' $(( d / 10 )) $(( d < 0 ? -d % 10 : d % 10 ))
}

read -r rq rs < build/simd-ref/bench-simd.txt
printf '\n%-10s %14s %9s %14s %9s\n' BUILD "quad 320x240" "vs ref" "sphere 40x40" "vs ref"
printf '%s\n' "-----------------------------------------------------------"
for row in "${trees[@]}"; do
  IFS='|' read -r name tree _ <<<"$row"; tree=${tree// /}
  read -r q s < "$tree/bench-simd.txt"
  printf '%-10s %14s %9s %14s %9s\n' "${name// /}" "$q" "$(pct "$q" "$rq")" "$s" "$(pct "$s" "$rs")"
done
echo
echo "ms/frame, best of $ROUNDS interleaved rounds. Lower is better."
