# The example programs. Every one is the same shape: a backend library
# implementing the five function pointers in render/backend.h, linked against
# the shared common/main.c.

function(add_pingo_example exe backend_lib backend_src)
    get_filename_component(backend_dir "${backend_src}" DIRECTORY)

    add_library(${backend_lib} SHARED ${backend_src})
    target_link_libraries(${backend_lib} PUBLIC pingo_render ${ARGN})
    target_include_directories(${backend_lib} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/${backend_dir})

    add_executable(${exe} common/main.c)
    target_link_libraries(${exe} PRIVATE pingo_assets ${backend_lib})
endfunction()

if(WIN32)
    add_pingo_example(win_window win32_window_backend
        win32_window/windowbackend.c gdi32)
endif()

# Needs no OS facilities beyond stdout, so it builds everywhere.
add_pingo_example(terminal terminal_backend terminal/terminalbackend.c)

# Needs linux/fb.h: Linux specifically, not merely a UNIX. NOT APPLE would
# still wrongly include the BSDs.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_pingo_example(linux_framebuffer linux_framebuffer_backend
        linux_framebuffer/linux_framebuffer_backend.c)
endif()

if(UNIX)

    if(X11_FOUND)
        add_pingo_example(linux_window linux_window_backend
            linux_window/linux_window_backend.c ${X11_LIBRARIES})
    else()
        message(STATUS "libX11 unavailable - skipping the linux_window example")
    endif()

    if(JPEG_FOUND)
        add_pingo_example(render_to_image jpeg_backend
            render_to_image/jpeg_backend.c JPEG::JPEG)
    else()
        message(STATUS "libjpeg unavailable - skipping the render_to_image example")
    endif()

    # Examples resolve "assets/viking.rgba" relative to the working directory,
    # so put a copy next to the binaries.
    configure_file(${PROJECT_SOURCE_DIR}/assets/viking.rgba
        ${CMAKE_BINARY_DIR}/assets/viking.rgba COPYONLY)
endif()

add_subdirectory(benchmarks)
