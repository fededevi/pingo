# Benchmarks that drive a real backend, rather than measuring maths in
# isolation - which is why they live with the examples and not under math/.
# Both open an X11 window, so neither exists without libX11.

if(NOT X11_FOUND)
    message(STATUS
        "libX11 unavailable - skipping visual_benchmarks and library_optimized_test")
    return()
endif()

add_executable(visual_benchmarks
    benchmark_visual_main.c
    benchmark_visual.c
    benchmark_timer.c)
target_include_directories(visual_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(visual_benchmarks PRIVATE pingo_assets linux_window_backend)

add_executable(library_optimized_test
    benchmark_library_optimized.c
    benchmark_timer.c)
target_include_directories(library_optimized_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(library_optimized_test PRIVATE pingo_assets linux_window_backend)
