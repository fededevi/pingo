add_subdirectory(unit)

# Golden-image tests for pingo_render.
#
# These render into memory rather than onto a screen, so they run everywhere -
# including the cross-compiled targets, where no window or framebuffer backend
# exists.

# Shared with assets/tests, which renders the shipped meshes through it.
add_library(pingo_memory_backend STATIC memory_backend.c golden.c)
target_include_directories(pingo_memory_backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pingo_memory_backend PUBLIC pingo_render)

add_executable(render_tests test_render.c)
target_link_libraries(render_tests PRIVATE pingo_memory_backend)

# The reference images are found through an absolute path rather than the
# working directory, so the test does not care where it is run from.
target_compile_definitions(render_tests PRIVATE
    PINGO_REFERENCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/references")

set(PINGO_RENDER_SCENES empty triangle cube no-uv group)

foreach(scene IN LISTS PINGO_RENDER_SCENES)
    add_test(NAME render.${scene} COMMAND render_tests ${scene})
    set_tests_properties(render.${scene} PROPERTIES TIMEOUT 30)
endforeach()

