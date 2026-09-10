# ---------------------------------------------------------------------------
# Which hand-written implementation of the span seam, if any, this build uses.
#
# Selection is compile-time and nothing else: no cpuid, no function pointers,
# no ifunc. The consequence is accepted - a binary runs only on the ISA it was
# built for - and the gain is that the dispatcher collapses to a direct call,
# LTO sees through it, and no hot path pays for an indirect branch.
#
# The C reference is always compiled, so a target with no hand-written path is
# never broken, only unaccelerated.
# ---------------------------------------------------------------------------

set(PINGO_SIMD "auto" CACHE STRING
    "Span implementation: auto (best for the target) or off (C reference)")
set_property(CACHE PINGO_SIMD PROPERTY STRINGS auto off)

# Ask the compiler for its target rather than reading it off its filename: the
# name need not contain the triple and frequently does not - a versioned
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
set(_pingo_simd_why "")

if(PINGO_SIMD STREQUAL "auto" AND PINGO_SIMD_TARGET_X86_64)
    # SSE2 needs no probe: it is part of the x86_64 ABI, so the compiler
    # defines __SSE2__ with no flags passed and there is nothing to fall back
    # to. AVX2 is different - it needs -mavx2, which yields a binary that will
    # not start on a machine without it, so it is never chosen automatically.
    # The avx2 preset asks for it by putting the flag in CMAKE_C_FLAGS.
    if(CMAKE_C_FLAGS MATCHES "-mavx2" OR CMAKE_C_FLAGS MATCHES "-march=x86-64-v3")
        set(_pingo_want "avx2")
    else()
        set(_pingo_want "sse2")
    endif()

    # Only select an implementation whose source is actually present. This
    # keeps the tree buildable while the assembly is being written, and means
    # a half-finished checkout degrades to the reference rather than failing
    # to configure. It is not a stub: absence selects the reference, which is
    # a real implementation.
    if(EXISTS "${PROJECT_SOURCE_DIR}/render/simd/span_${_pingo_want}.S")
        set(PINGO_SIMD_SELECTED "${_pingo_want}")
    else()
        set(_pingo_simd_why
            " (render/simd/span_${_pingo_want}.S not present yet)")
    endif()
endif()

if(NOT PINGO_SIMD_SELECTED STREQUAL "reference")
    # Only now, so no target without a hand-written path makes CMake go
    # looking for an assembler it will never use.
    enable_language(ASM)
endif()

message(STATUS "pingo: span implementation is ${PINGO_SIMD_SELECTED}"
               "${_pingo_simd_why} (target ${_pingo_triple})")
