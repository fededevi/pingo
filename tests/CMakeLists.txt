# Test suite for Pingo Math Library

# Find math library (for M_PI constant)
find_library(MATH_LIBRARY m)

# Create test executable
add_executable(math_tests
    main.c
    test_runner.c
    test_vec2.c
    test_vec3.c
    test_vec4.c
    test_mat3.c
    test_mat4.c
    test_functions.c
)

# Create benchmark executable
add_executable(math_benchmarks
    benchmark_main.c
    benchmark_utils.c
    benchmark_vec2.c
    benchmark_vec3.c
    benchmark_mat3.c
    benchmark_mat4.c
    benchmark_functions.c
)

# Create visual benchmark executable (uses real renderer)
add_executable(visual_benchmarks
    benchmark_visual_main.c
    benchmark_visual.c
    benchmark_simple_utils.c
    ../example/linux_window/linux_window_backend.c
)

# Create library optimization test executable
add_executable(library_optimized_test
    benchmark_library_optimized.c
    benchmark_simple_utils.c
    ../example/linux_window/linux_window_backend.c
)

# Include directories
target_include_directories(math_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(math_benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(visual_benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/example
    ${CMAKE_SOURCE_DIR}/render
)

target_include_directories(library_optimized_test PRIVATE
    ${CMAKE_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/example
    ${CMAKE_SOURCE_DIR}/render
)

# Link libraries
target_link_libraries(math_tests
    ${MATH_LIBRARY}
    pingo
    assets
)

# Link libraries for non-X11 targets
target_link_libraries(math_tests
    ${MATH_LIBRARY}
    pingo
    assets
)

target_link_libraries(math_benchmarks
    ${MATH_LIBRARY}
    pingo
    assets
)

# Handle X11-dependent targets
if(UNIX AND NOT APPLE)
    find_package(X11)
    if(X11_FOUND)
        message(STATUS "X11 found - building visual benchmarks")
        target_link_libraries(visual_benchmarks
            ${MATH_LIBRARY}
            pingo
            assets
            ${X11_LIBRARIES}
        )
        target_link_libraries(library_optimized_test
            ${MATH_LIBRARY}
            pingo
            assets
            ${X11_LIBRARIES}
        )
    else()
        message(WARNING "X11 not found - disabling visual benchmarks")
        # Disable X11-dependent targets
        set_target_properties(visual_benchmarks PROPERTIES EXCLUDE_FROM_ALL TRUE)
        # Library test can still work without X11 (just won't render visually)
        target_link_libraries(library_optimized_test
            ${MATH_LIBRARY}
            pingo
            assets
        )
    endif()
else()
    # Non-Unix systems - disable visual benchmarks, enable library test without X11
    message(STATUS "Non-Unix system - visual benchmarks disabled")
    set_target_properties(visual_benchmarks PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_link_libraries(library_optimized_test
        ${MATH_LIBRARY}
        pingo
        assets
    )
endif()

# Add test to CTest
add_test(NAME MathLibraryTests COMMAND math_tests)

# Set test properties
set_tests_properties(MathLibraryTests PROPERTIES
    TIMEOUT 30
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
