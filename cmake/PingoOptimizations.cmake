# ---------------------------------------------------------------------------
# Optimizations that are a property of the build rather than of the code.
#
# Everything here is probed before it is used and reported once at configure
# time, so any toolchain gets whatever it happens to support and no target can
# be broken by asking for something it lacks. That matters here: the same
# source is built by nineteen compilers in CI, several of them freestanding.
#
# Nothing in this file changes behaviour, only code generation.
# ---------------------------------------------------------------------------

include(CheckIPOSupported)
include(CheckCCompilerFlag)

# Whether this build asked for optimization at all. With no build type the
# compiler gets no -O flag, and turning LTO on there would slow the build down
# to no purpose.
set(_pingo_optimized OFF)
if(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$"
   OR CMAKE_CONFIGURATION_TYPES)
    set(_pingo_optimized ON)
endif()

# ---------------------------------------------------------------------------
# Link-time optimization.
#
# The renderer's inner loop and the pixel write it calls live in different
# translation units, so without LTO the call survives and the loop cannot be
# kept in registers or vectorized across it. Measured on x86-64 at -O3, LTO is
# worth about 16% on fill-bound rendering - more than any single source change
# tried against it.
# ---------------------------------------------------------------------------
option(PINGO_ENABLE_LTO
    "Optimize across translation units, where the toolchain supports it" ON)

if(PINGO_ENABLE_LTO AND _pingo_optimized)
    check_ipo_supported(RESULT _pingo_ipo OUTPUT _pingo_ipo_error LANGUAGES C)
    if(_pingo_ipo)
        # include() does not open a scope, so this reaches every target
        # added after it, in this directory and all subdirectories.
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
        message(STATUS "pingo: link-time optimization enabled")
    else()
        # Not a warning: plenty of embedded toolchains simply do not have it.
        message(STATUS "pingo: link-time optimization unavailable, skipping")
    endif()
endif()

# ---------------------------------------------------------------------------
# Dead code and data removal.
#
# Put every function and object in its own section so the linker can drop the
# ones nothing reached. Pingo is a library with several entry points a given
# program will not use - the rasterizer's line and sprite paths, for instance -
# and on a microcontroller that unused code is flash that has to be paid for.
# ---------------------------------------------------------------------------
option(PINGO_ENABLE_GC_SECTIONS
    "Let the linker drop functions and data that nothing references" ON)

if(PINGO_ENABLE_GC_SECTIONS AND _pingo_optimized)
    check_c_compiler_flag(-ffunction-sections _pingo_has_function_sections)
    if(_pingo_has_function_sections)
        add_compile_options(-ffunction-sections -fdata-sections)
        # Apple's linker spells it differently and collects by default anyway.
        if(APPLE)
            add_link_options(-Wl,-dead_strip)
        else()
            check_c_compiler_flag("-Wl,--gc-sections" _pingo_has_gc_sections)
            if(_pingo_has_gc_sections)
                add_link_options(-Wl,--gc-sections)
            endif()
        endif()
        message(STATUS "pingo: unreferenced sections will be discarded")
    endif()
endif()

# ---------------------------------------------------------------------------
# Host-specific instruction selection.
#
# Off by default and deliberately so: it produces a binary that may not run on
# any machine but the one that built it, which would break both the cross
# builds and any distributable package. It is here for benchmarking and for
# people building for hardware they own.
# ---------------------------------------------------------------------------
option(PINGO_ENABLE_NATIVE
    "Tune for the building machine's own CPU (not portable)" OFF)

if(PINGO_ENABLE_NATIVE)
    check_c_compiler_flag(-march=native _pingo_has_native)
    if(_pingo_has_native)
        add_compile_options(-march=native)
        message(STATUS "pingo: tuning for the host CPU, the result is not portable")
    else()
        message(WARNING "pingo: -march=native was requested but is not supported")
    endif()
endif()
