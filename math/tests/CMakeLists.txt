# Tests and benchmarks for pingo_math.
#
# One CTest entry per suite rather than a single opaque one, so a failure
# names the suite that broke and they can run in parallel. math_tests with no
# argument still runs the lot.

add_executable(math_tests
    main.c
    test_runner.c
    test_vec2.c
    test_vec3.c
    test_vec4.c
    test_mat3.c
    test_mat4.c
    test_functions.c)
target_include_directories(math_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(math_tests PRIVATE pingo_math)

add_executable(math_benchmarks
    benchmark_main.c
    benchmark_utils.c
    benchmark_vec2.c
    benchmark_vec3.c
    benchmark_mat3.c
    benchmark_mat4.c
    benchmark_functions.c)
target_include_directories(math_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(math_benchmarks PRIVATE pingo_math)

# Benchmark numbers from an unoptimized build are meaningless, and the reason
# is not obvious from the output - so say so here rather than forcing a build
# type on the whole project.
if(NOT CMAKE_CONFIGURATION_TYPES AND
   (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(STATUS
        "math_benchmarks will measure unoptimized code: CMAKE_BUILD_TYPE is "
        "'${CMAKE_BUILD_TYPE}'. Use Release for meaningful numbers.")
endif()

set(PINGO_MATH_SUITES vec2 vec3 vec4 mat3 mat4 functions)

foreach(suite IN LISTS PINGO_MATH_SUITES)
    add_test(NAME math.${suite} COMMAND math_tests ${suite})
    set_tests_properties(math.${suite} PROPERTIES TIMEOUT 30)
endforeach()
