# The maths: vectors and matrices. Depends on nothing but libm.

file(GLOB_RECURSE math_src CONFIGURE_DEPENDS *.c)
list(FILTER math_src EXCLUDE REGEX "/tests/")

add_library(pingo_math ${math_src})

target_include_directories(pingo_math PUBLIC
    # Sources refer to each other both as "vec3.h" and as "math/vec3.h".
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pingo>)

set_target_properties(pingo_math PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

if(UNIX)
    target_link_libraries(pingo_math PUBLIC m)
endif()

# The SIMD selection, as PUBLIC definitions here rather than on pingo_render:
# mat4.h has a vector body behind PINGO_SIMD_AVX2, and this is the library
# that compiles and tests it. pingo_render links pingo_math PUBLIC, so the
# definitions reach it and everything downstream from one place. Without this
# math_tests would silently test the scalar transform in an AVX2 build.
if(PINGO_SIMD_SELECTED STREQUAL "avx2")
    target_compile_definitions(pingo_math PUBLIC PINGO_SIMD_AVX2=1 PINGO_SIMD_SSE2=1)
elseif(PINGO_SIMD_SELECTED STREQUAL "sse2")
    target_compile_definitions(pingo_math PUBLIC PINGO_SIMD_SSE2=1)
endif()

install(TARGETS pingo_math
    EXPORT pingoTargets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo)

install(DIRECTORY ./
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo/math
    FILES_MATCHING PATTERN "*.h")

add_subdirectory(tests)
