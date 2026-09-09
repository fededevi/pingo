# The renderer: rasterizer, scene graph, textures, depth buffer.

file(GLOB_RECURSE render_src CONFIGURE_DEPENDS *.c)
list(FILTER render_src EXCLUDE REGEX "/tests/")

add_library(pingo_render ${render_src})

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
