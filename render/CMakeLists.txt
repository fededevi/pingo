# The renderer: rasterizer, scene graph, textures, depth buffer.

file(GLOB_RECURSE render_src CONFIGURE_DEPENDS *.c)
list(FILTER render_src EXCLUDE REGEX "/(tests|benchmarks)/")

add_library(pingo_render ${render_src})

# The hand-written span, when this build selected one. PUBLIC on the compile
# definition because span.h is installed and its inline dispatcher must resolve
# the same way for consumers as it does here - otherwise a caller would inline
# a call to a symbol this library does not export.
if(NOT PINGO_SIMD_SELECTED STREQUAL "reference")
    target_sources(pingo_render PRIVATE simd/span_${PINGO_SIMD_SELECTED}.S)
    string(TOUPPER "${PINGO_SIMD_SELECTED}" _pingo_simd_upper)
    target_compile_definitions(pingo_render
        PUBLIC PINGO_SIMD_${_pingo_simd_upper}=1)
endif()

# An AVX2 build also carries the SSE2 span, and span.h dispatches between them
# on the run's length: the eight-wide path has a fixed per-span cost that a
# short run does not amortise, and it was measured 22% slower than SSE2 on a
# mesh whose rows are mostly 16-40 pixels. Still compile-time, still a direct
# call - the choice is one compare on a value the caller already holds.
if(PINGO_SIMD_SELECTED STREQUAL "avx2")
    target_sources(pingo_render PRIVATE simd/span_sse2.S)
    target_compile_definitions(pingo_render PUBLIC PINGO_SIMD_SSE2=1)
endif()

target_include_directories(pingo_render PUBLIC
    # Sources refer to each other both as "pixel.h" and as "render/pixel.h".
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pingo>)

# PUBLIC: the render headers expose Vec3f, Mat4 and friends, so anything
# linking the renderer needs the maths too.
target_link_libraries(pingo_render PUBLIC pingo_math)

set_target_properties(pingo_render PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS pingo_render
    EXPORT pingoTargets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo)

install(DIRECTORY ./
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo/render
    FILES_MATCHING PATTERN "*.h")

add_subdirectory(tests)
add_subdirectory(benchmarks)
