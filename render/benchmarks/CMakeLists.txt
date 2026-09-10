# Frame-rate benchmark. Not registered with CTest: it reports a rate rather
# than passing or failing, and timings vary too much between runners to gate on.

add_executable(render_benchmarks benchmark_render.c)
target_link_libraries(render_benchmarks PRIVATE pingo_memory_backend)

# ---------------------------------------------------------------------------
# Running the benchmark from the build system.
#
# These cover the ordinary case - measuring the tree you are already in. The
# scripts in scripts/ remain for the two things a target cannot do: comparing
# separate build trees built by different compilers (bench-all-arch.sh), and
# putting the machine into a repeatable state, which needs root and touches
# sysfs, cgroups, IRQ affinity and CPU hotplug (bench-cpu.sh).
# ---------------------------------------------------------------------------
set(PINGO_BENCH_FRAMES 100 CACHE STRING "Frames per case for the bench target")
set(PINGO_BENCH_REPEAT 5 CACHE STRING "Timed repeats per case; the best is reported")
set(PINGO_BENCH_CPU "" CACHE STRING
    "Pin bench runs to this CPU. Reserve it first with scripts/bench-cpu.sh lock, \
otherwise pinning alone still leaves the clock free to roam.")

# Pinning is decided here rather than inside the benchmark: which core is
# reserved is a property of the machine, not of the code being measured.
set(_bench_prefix "")
if(PINGO_BENCH_CPU)
    find_program(TASKSET_EXECUTABLE taskset)
    if(TASKSET_EXECUTABLE)
        set(_bench_prefix ${TASKSET_EXECUTABLE} -c ${PINGO_BENCH_CPU})
        message(STATUS "pingo: bench targets will run on cpu ${PINGO_BENCH_CPU}")
    else()
        message(WARNING "pingo: PINGO_BENCH_CPU is set but taskset was not found")
    endif()
endif()

# A cross-built binary runs under qemu's binfmt handler, which finds no loader
# without QEMU_LD_PREFIX pointing at the matching sysroot.
#
# Ask the compiler for its target rather than reading it off its filename.
# The name is not required to contain the triple and frequently does not:
# a versioned aarch64-linux-gnu-gcc-11, a plain "gcc", "cc", a ccache wrapper,
# or clang driven by --target= all defeat any amount of string surgery, and
# several such compilers are installed here. -dumpmachine is the compiler's
# own answer and also reflects the flags it was given.
#
# Deliberately not guarded by CMAKE_CROSSCOMPILING: the arch-* presets set only
# CC, never CMAKE_SYSTEM_NAME, so CMake classifies every one of them as a native
# build and that variable is not even written to the cache. Guarding on it
# silently produced a binary that could not start and a blank timing column.
# For a genuinely native build the variable is inert - only qemu reads it - so
# the existence of the sysroot is a sufficient test.
execute_process(
    COMMAND "${CMAKE_C_COMPILER}" -dumpmachine
    OUTPUT_VARIABLE _triple
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _dumpmachine_rc)

if(NOT _dumpmachine_rc EQUAL 0)
    # MSVC and a few others have no -dumpmachine. Nothing here needs an
    # emulator on those, so falling back to no prefix is the right answer.
    set(_triple "")
endif()

set(_bench_env "")
if(_triple AND IS_DIRECTORY "/usr/${_triple}")
    set(_bench_env ${CMAKE_COMMAND} -E env "QEMU_LD_PREFIX=/usr/${_triple}")
    message(STATUS "pingo: bench targets use sysroot /usr/${_triple} (target ${_triple})")
endif()

# USES_TERMINAL: the benchmark prints a table as it goes, and without this
# the generator buffers it until the run ends.
add_custom_target(bench
    COMMAND ${_bench_env} ${_bench_prefix} $<TARGET_FILE:render_benchmarks>
            ${PINGO_BENCH_FRAMES} --repeat ${PINGO_BENCH_REPEAT}
    DEPENDS render_benchmarks
    COMMENT "Rendering ${PINGO_BENCH_FRAMES} frames per case, best of ${PINGO_BENCH_REPEAT}"
    USES_TERMINAL
    VERBATIM)

# The cross-architecture script reads this file and nothing else - no parsing,
# no field selection, no skipping of progress lines. Which cases are headline
# is decided in the benchmark source, next to the case table.
set(PINGO_BENCH_SUMMARY ${CMAKE_BINARY_DIR}/bench-summary.txt)

add_custom_target(bench-summary
    COMMAND ${_bench_env} ${_bench_prefix} $<TARGET_FILE:render_benchmarks>
            ${PINGO_BENCH_FRAMES} --repeat ${PINGO_BENCH_REPEAT}
            --summary ${PINGO_BENCH_SUMMARY}
    DEPENDS render_benchmarks
    COMMENT "Benchmark, headline cases only"
    USES_TERMINAL
    VERBATIM)

add_custom_target(bench-tsv
    COMMAND ${_bench_env} ${_bench_prefix} $<TARGET_FILE:render_benchmarks>
            ${PINGO_BENCH_FRAMES} --repeat ${PINGO_BENCH_REPEAT} --tsv
    DEPENDS render_benchmarks
    COMMENT "Benchmark, machine-readable"
    USES_TERMINAL
    VERBATIM)

# Answers "can this machine measure anything right now", which is worth
# knowing before trusting an A/B - and is the check that fails loudly when
# no core has been reserved.
add_custom_target(bench-stability
    COMMAND ${_bench_env} ${_bench_prefix} $<TARGET_FILE:render_benchmarks>
            60 --repeat 12 --stability
    DEPENDS render_benchmarks
    COMMENT "Checking measurement stability"
    USES_TERMINAL
    VERBATIM)
