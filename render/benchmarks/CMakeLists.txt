# Frame-rate benchmark. Not registered with CTest: it reports a rate rather
# than passing or failing, and timings vary too much between runners to gate on.

add_executable(render_benchmarks benchmark_render.c)
target_link_libraries(render_benchmarks PRIVATE pingo_memory_backend)
