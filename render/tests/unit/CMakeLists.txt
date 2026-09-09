# Unit tests for the render types.
#
# The golden-image tests next door check what the renderer draws; these check
# the pieces in isolation, which is where a rejected argument or an off-by-one
# index shows up as a clear failure rather than a shifted pixel.

add_executable(render_unit_tests
    main.c
    test_unit_runner.c
    test_pixel.c
    test_texture.c
    test_depth.c
    test_target.c
    test_transform.c
    test_object_sprite_material.c
    test_renderer.c)
target_include_directories(render_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(render_unit_tests PRIVATE pingo_render)

set(PINGO_RENDER_UNIT_SUITES
    pixel texture depth target transform drawables renderer)

foreach(suite IN LISTS PINGO_RENDER_UNIT_SUITES)
    add_test(NAME render.unit.${suite} COMMAND render_unit_tests ${suite})
    set_tests_properties(render.unit.${suite} PROPERTIES TIMEOUT 30)
endforeach()
