# backends/CMakeLists.txt

# Helper for common include dirs
set(PINGO_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/render
    ${CMAKE_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/assets
)

# Linux window backend
add_library(backend_linux_window SHARED
    linux_window/linux_window_backend.c
)
target_include_directories(backend_linux_window PUBLIC linux_window ${PINGO_INCLUDE_DIRS})
target_link_libraries(backend_linux_window PUBLIC pingo)

# Linux framebuffer backend
add_library(backend_linux_framebuffer SHARED
    linux_framebuffer/linux_framebuffer_backend.c
)
target_include_directories(backend_linux_framebuffer PUBLIC linux_framebuffer ${PINGO_INCLUDE_DIRS})
target_link_libraries(backend_linux_framebuffer PUBLIC pingo)

# Terminal backend
add_library(backend_terminal SHARED
    terminal/terminalbackend.c
)
target_include_directories(backend_terminal PUBLIC terminal ${PINGO_INCLUDE_DIRS})
target_link_libraries(backend_terminal PUBLIC pingo)

# Render-to-image backend (JPEG)
add_library(backend_render_to_image SHARED
    render_to_image/jpeg_backend.c
)
target_include_directories(backend_render_to_image PUBLIC render_to_image ${PINGO_INCLUDE_DIRS})
target_link_libraries(backend_render_to_image PUBLIC pingo)

# Win32 backend (only on Windows)
if(WIN32)
    add_library(backend_win32_window SHARED
        win32_window/windowbackend.c
    )
    target_include_directories(backend_win32_window PUBLIC win32_window ${PINGO_INCLUDE_DIRS})
    target_link_libraries(backend_win32_window PUBLIC pingo)
endif()
