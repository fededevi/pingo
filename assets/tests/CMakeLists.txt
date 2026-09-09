# Golden-image tests for the shipped meshes. They render through the memory
# backend from render/tests, so they need no window, no libjpeg and no X11 and
# therefore run on every platform CI builds for.

add_executable(asset_tests test_assets.c)
target_link_libraries(asset_tests PRIVATE pingo_assets pingo_memory_backend)

target_compile_definitions(asset_tests PRIVATE
    PINGO_REFERENCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/references")

set(PINGO_ASSET_MESHES cube teapot viking pingo)

foreach(mesh IN LISTS PINGO_ASSET_MESHES)
    add_test(NAME assets.${mesh} COMMAND asset_tests ${mesh})
    set_tests_properties(assets.${mesh} PROPERTIES TIMEOUT 60)
endforeach()
