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
# but the host runs under emulation.
#
set -uo pipefail
cd "$(dirname "$0")/.."

FRAMES=${FRAMES:-20}
FILTERS=("$@")

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

  if ! out=$(cmake --workflow --preset "$p" 2>&1); then
    echo "$out" > "$log"
    # Tell a toolchain that is not installed apart from one that is broken.
    if echo "$out" | grep -qiE "is not able to compile|CMAKE_C_COMPILER.*not found|No such file or directory.*gcc"; then
      printf '%-22s %-7s %s\n' "$name" "-" "toolchain missing or cannot link"
    elif echo "$out" | grep -q "tests failed"; then
      n=$(echo "$out" | grep -oE '[0-9]+ tests failed' | head -1)
      printf '%-22s %-7s %s\n' "$name" "-" "$n (see $log)"
    else
      printf '%-22s %-7s %s\n' "$name" "-" "build failed (see $log)"
    fi
    bad+=1
    continue
  fi

  if echo "$out" | grep -q '100% tests passed'; then
    n=$(echo "$out" | grep -oE 'out of [0-9]+' | tail -1 | grep -oE '[0-9]+')
    detail="$n/$n tests passed"
  else
    detail="built, nothing to run (freestanding)"
  fi

  bin="build/$p/render_benchmarks"
  if [ ! -x "$bin" ]; then
    cmake --build "build/$p" --target render_benchmarks -j"$(nproc)" >>"$log" 2>&1
  fi

  # The preset's environment applies to CMake's own processes, not to a binary
  # this script execs itself, and qemu's binfmt handler finds no loader without
  # it. Read it back out of the preset rather than restating the sysroot here.
  prefix=$(python3 -c "
import json, sys
d = json.load(open('CMakePresets.json'))
for cp in d.get('configurePresets', []):
    if cp['name'] == sys.argv[1]:
        print(cp.get('environment', {}).get('QEMU_LD_PREFIX', ''))
        break
" "$p" 2>/dev/null)

  if [ -x "$bin" ] && ms=$(QEMU_LD_PREFIX="$prefix" "$bin" "$FRAMES" 2>/dev/null | awk -v f="$FRAMES" '
        /^  quad .*320x240/ { for(i=1;i<=NF;i++) if($i=="s"){q=$(i-1);break} }
        /^  sphere 40x40/   { for(i=1;i<=NF;i++) if($i=="s"){s=$(i-1);break} }
        END { if (q!="") printf "%.2f %.2f", q*1000/f, s*1000/f }') && [ -n "$ms" ]; then
    read -r q s <<<"$ms"
    MS["$name"]="$q $s"
    detail="$detail, ${q} ms/frame"
  fi

  printf '%-22s %-7s %s\n' "$name" "+" "$detail"
  ok+=1
done

printf '%s\n' "----------------------------------------------------------------------"
printf '%d +   %d -   (%d total)\n' "$ok" "$bad" $((ok + bad))

if [ ${#MS[@]} -gt 1 ]; then
  echo
  echo "Render benchmark, $FRAMES frames per case, ms/frame (lower is better)."
  echo "Only the host runs natively; the rest are emulated, so read these as a"
  echo "check that each architecture works, not as hardware performance."
  printf '%-22s %14s %14s %10s\n' TARGET "quad 320x240" "sphere 40x40" "vs host"
  printf '%s\n' "------------------------------------------------------------------"
  host=${MS["x86_64 (host)"]:-}
  hq=$(echo "$host" | cut -d' ' -f1)
  for k in "${!MS[@]}"; do
    read -r q s <<<"${MS[$k]}"
    rel=""
    [ -n "$hq" ] && rel=$(awk -v a="$q" -v b="$hq" 'BEGIN{printf "%.0fx", a/b}')
    printf '%-22s %14s %14s %10s\n' "$k" "$q" "$s" "$rel"
  done | sort -t' ' -k1,1
fi

[ "$bad" -eq 0 ]
