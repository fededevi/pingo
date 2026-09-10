#!/usr/bin/env bash
#
# Reserve one P-core at a fixed clock, for benchmarks that can be compared
# across sessions.
#
# The host is a hybrid part: CPUs 0-3 are P-cores (Raptor Cove, up to 5.0 GHz,
# SMT pairs 0-1 and 2-3), CPUs 4-11 are E-cores (Gracemont, up to 3.7 GHz).
# A run that is free to migrate between the two is comparing microarchitectures
# rather than builds, and interleaving the runs does not recover from that.
#
# Four separate things have to be nailed down, and doing only some of them
# leaves the drift in place:
#
#   clock     the core is pinned min=max, so it cannot ramp or turbo
#   sibling   the SMT twin is taken offline, so the physical core is not shared
#   tasks     system.slice and init.scope are evicted from the core
#   IRQs      interrupt affinity is steered away from it
#
# Only the bench core is touched. The rest of the machine keeps its governor
# and its turbo, because making the whole laptop slow to time one core is a
# poor trade - and a global no_turbo would do exactly that.
#
#   sudo ./scripts/bench-cpu.sh lock [MHz]   reserve the core (default 2000)
#   sudo ./scripts/bench-cpu.sh unlock       give it all back
#        ./scripts/bench-cpu.sh status       what is set right now
#        ./scripts/bench-cpu.sh verify [n]   measure the residual noise
#        ./scripts/bench-cpu.sh run -- cmd   run cmd on the reserved core
#
# user.slice deliberately keeps access to the core: a cpuset that excluded it
# would make `taskset -c N` fail for anything the user launches, and then every
# benchmark run would need root. Evicting the daemons and the IRQs is what
# actually removes the jitter; the desktop will schedule elsewhere on its own
# while eleven other CPUs sit free.
#
set -uo pipefail

CPUDIR=/sys/devices/system/cpu
CG=/sys/fs/cgroup
STATE=/var/tmp/pingo-bench-cpu.state
CPU=${PINGO_BENCH_CPU:-2}          # a P-core; CPU 0 tends to collect interrupts
SLICES=(system.slice init.scope)

die() { echo "error: $*" >&2; exit 1; }
need_root() { [ "$(id -u)" -eq 0 ] || die "'$1' needs root - re-run with sudo"; }

sibling_of() { # the other SMT thread on the same physical core, if any
  # An offline CPU leaves the sibling list, so once this script has taken the
  # twin down the topology no longer names it. The value recorded at lock time
  # is the authority from then on - without this, status reported no sibling
  # at all for a core whose sibling it had just offlined.
  if [ -f "$STATE" ]; then
    local saved; saved=$(sed -n 's/^sibling=//p' "$STATE")
    [ -n "$saved" ] && { echo "$saved"; return; }
  fi
  local l; l=$(cat "$CPUDIR/cpu$1/topology/thread_siblings_list" 2>/dev/null)
  echo "${l//$1/}" | tr -d ',-' | tr -d ' '
}

all_cpus() { ls -d "$CPUDIR"/cpu[0-9]* | sed 's#.*/cpu##' | sort -n; }

# Every CPU except the reserved one, as a cpuset list: "0-1,3-11"
others_list() {
  local out="" c
  for c in $(all_cpus); do [ "$c" = "$CPU" ] || out="$out,$c"; done
  echo "${out#,}"
}

# Hex mask with the reserved CPU's bit cleared, for /proc/irq/*/smp_affinity
others_mask() {
  local m=0 c
  for c in $(all_cpus); do [ "$c" = "$CPU" ] || m=$(( m | (1 << c) )); done
  printf '%x\n' "$m"
}

cmd_lock() {
  need_root lock
  local mhz=${1:-2000} khz d lo hi t sib
  khz=$(( mhz * 1000 ))
  d=$CPUDIR/cpu$CPU/cpufreq
  [ -d "$d" ] || die "cpu$CPU has no cpufreq directory"

  sib=$(sibling_of "$CPU")

  if [ ! -f "$STATE" ]; then
    { echo "cpu=$CPU"
      echo "governor=$(cat "$d"/scaling_governor)"
      echo "min=$(cat "$d"/scaling_min_freq)"
      echo "max=$(cat "$d"/scaling_max_freq)"
      echo "sibling=$sib"
      echo "sibling_online=$([ -n "$sib" ] && cat "$CPUDIR/cpu$sib/online" 2>/dev/null || echo -)"
      echo "default_irq=$(cat /proc/irq/default_smp_affinity)"
      for s in "${SLICES[@]}"; do
        [ -f "$CG/$s/cpuset.cpus" ] && echo "slice:$s=$(cat "$CG/$s/cpuset.cpus")"
      done
    } > "$STATE"
  fi

  # 1. Clock. Widen before narrowing: writing max below the standing min is
  #    rejected, and so is the reverse.
  lo=$(cat "$d"/cpuinfo_min_freq); hi=$(cat "$d"/cpuinfo_max_freq)
  t=$khz; [ "$t" -lt "$lo" ] && t=$lo; [ "$t" -gt "$hi" ] && t=$hi
  echo performance > "$d"/scaling_governor 2>/dev/null
  echo "$lo" > "$d"/scaling_min_freq
  echo "$t"  > "$d"/scaling_max_freq
  echo "$t"  > "$d"/scaling_min_freq

  # 2. SMT sibling offline, so nothing else runs on the same physical core.
  if [ -n "$sib" ] && [ -w "$CPUDIR/cpu$sib/online" ]; then
    echo 0 > "$CPUDIR/cpu$sib/online" && echo "cpu$sib (SMT sibling) offlined"
  fi

  # 3. Evict the system daemons. Writing a cpuset migrates the tasks already
  #    in the slice, it does not merely constrain the next ones.
  local others; others=$(others_list)
  for s in "${SLICES[@]}"; do
    [ -f "$CG/$s/cpuset.cpus" ] && echo "$others" > "$CG/$s/cpuset.cpus" 2>/dev/null
  done

  # 4. Steer interrupts away. Many IRQs refuse a new mask (per-CPU timers,
  #    managed MSI queues); that is expected, so failures are counted and not
  #    reported one by one.
  local mask moved=0 pinned=0
  mask=$(others_mask)
  echo "$mask" > /proc/irq/default_smp_affinity 2>/dev/null
  for i in /proc/irq/[0-9]*; do
    if echo "$mask" > "$i"/smp_affinity 2>/dev/null; then moved=$((moved+1)); else pinned=$((pinned+1)); fi
  done

  echo "cpu$CPU reserved at ${mhz} MHz; ${moved} IRQs steered away, ${pinned} could not move"
  echo "run benchmarks with: taskset -c $CPU <cmd>   (or $0 run -- <cmd>)"
  echo "release it with: sudo $0 unlock"
}

cmd_unlock() {
  need_root unlock
  [ -f "$STATE" ] || die "no saved state at $STATE, nothing to restore"
  local cpu gov mn mx sib sib_on dirq d
  cpu=$(sed -n 's/^cpu=//p' "$STATE")
  gov=$(sed -n 's/^governor=//p' "$STATE")
  mn=$(sed -n 's/^min=//p' "$STATE")
  mx=$(sed -n 's/^max=//p' "$STATE")
  sib=$(sed -n 's/^sibling=//p' "$STATE")
  sib_on=$(sed -n 's/^sibling_online=//p' "$STATE")
  dirq=$(sed -n 's/^default_irq=//p' "$STATE")
  d=$CPUDIR/cpu$cpu/cpufreq

  echo "$(cat "$d"/cpuinfo_min_freq)" > "$d"/scaling_min_freq
  echo "$mx" > "$d"/scaling_max_freq
  echo "$mn" > "$d"/scaling_min_freq
  echo "$gov" > "$d"/scaling_governor 2>/dev/null

  [ "$sib_on" = "1" ] && [ -w "$CPUDIR/cpu$sib/online" ] && echo 1 > "$CPUDIR/cpu$sib/online"

  sed -n 's/^slice://p' "$STATE" | while IFS='=' read -r s v; do
    [ -f "$CG/$s/cpuset.cpus" ] && echo "$v" > "$CG/$s/cpuset.cpus" 2>/dev/null
  done

  echo "$dirq" > /proc/irq/default_smp_affinity 2>/dev/null
  for i in /proc/irq/[0-9]*; do echo "$dirq" > "$i"/smp_affinity 2>/dev/null; done

  rm -f "$STATE"
  echo "cpu$cpu released, governor=$gov, sibling restored"
}

cmd_status() {
  local d=$CPUDIR/cpu$CPU/cpufreq sib
  sib=$(sibling_of "$CPU")
  printf 'reserved cpu   %s (%s)\n' "$CPU" "$([ -f "$STATE" ] && echo LOCKED || echo not locked)"
  printf 'governor       %s\n' "$(cat "$d"/scaling_governor 2>/dev/null)"
  printf 'clock          %s - %s MHz (now %s)\n' \
    $(( $(cat "$d"/scaling_min_freq) / 1000 )) \
    $(( $(cat "$d"/scaling_max_freq) / 1000 )) \
    $(( $(cat "$d"/scaling_cur_freq) / 1000 ))
  printf 'SMT sibling    cpu%s, online=%s\n' "$sib" "$(cat "$CPUDIR/cpu$sib/online" 2>/dev/null || echo n/a)"
  for s in "${SLICES[@]}"; do
    printf '%-14s %s\n' "$s" "$(cat "$CG/$s/cpuset.cpus" 2>/dev/null || echo '(unset = all)')"
  done
  local n; n=$(grep -lx "$(others_mask)" /proc/irq/[0-9]*/smp_affinity 2>/dev/null | wc -l)
  printf 'IRQs steered   %s\n' "$n"
  echo
  printf '%-5s %-9s %-9s %-6s %s\n' CPU MAX_MHz CUR_MHz TYPE ''
  for c in $(all_cpus); do
    local dd=$CPUDIR/cpu$c/cpufreq mx
    [ -d "$dd" ] || { printf '%-5s %-9s %-9s %-6s %s\n' "$c" - - - OFFLINE; continue; }
    mx=$(cat "$dd"/cpuinfo_max_freq)
    printf '%-5s %-9s %-9s %-6s %s\n' "$c" $((mx/1000)) \
      $(( $(cat "$dd"/scaling_cur_freq) / 1000 )) \
      "$([ "$mx" -gt 4000000 ] && echo P || echo E)" \
      "$([ "$c" = "$CPU" ] && echo '<- bench')"
  done
}

cmd_run() {
  [ "${1:-}" = "--" ] && shift
  [ $# -gt 0 ] || die "run needs a command"
  [ -f "$STATE" ] || echo "warning: cpu$CPU is not locked, results will drift" >&2
  exec taskset -c "$CPU" "$@"
}

# The measurement that counts. scaling_cur_freq only reports what was asked
# for - it cannot see thermal throttling, which needs MSRs and root.
#
# The workload is the real render benchmark, compiled, repeating its timed
# loop inside one process. It used to be an interpreted loop here, which timed
# the interpreter's own jitter and startup alongside the machine's - on a
# quiet box the larger of the two, and exactly the quantity this is meant to
# measure. The statistics are computed in the binary for the same reason.
cmd_verify() {
  local repeat=${1:-12} bin=build/default/render_benchmarks
  cd "$(dirname "$0")/.." || die "cannot find the repository root"

  if [ ! -x "$bin" ]; then
    echo "building $bin ..."
    cmake --preset default >/dev/null 2>&1
    cmake --build build/default --target render_benchmarks -j"$(nproc)" >/dev/null 2>&1 \
      || die "could not build the benchmark"
  fi

  [ -f "$STATE" ] || echo "warning: cpu$CPU is not locked - measuring anyway" >&2
  taskset -c "$CPU" "$bin" 60 --repeat "$repeat" --stability
}

case "${1:-status}" in
  lock)   shift; cmd_lock "$@" ;;
  unlock) cmd_unlock ;;
  status) cmd_status ;;
  verify) shift; cmd_verify "$@" ;;
  run)    shift; cmd_run "$@" ;;
  *) echo "usage: $0 {lock [MHz]|unlock|status|verify [runs]|run -- <cmd>}" >&2; exit 2 ;;
esac
