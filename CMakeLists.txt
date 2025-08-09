cmake_minimum_required(VERSION 3.10)
project( pingo)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory and run cmake from there.")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options( -Wall -Wextra -Wpointer-arith )
endif()

if (MSVC)
    add_compile_options(
        /wd4305  # truncation from double to float
        /wd4244  # conversion possible loss of data
    )
endif()

# Pingo rendering library
file( GLOB_RECURSE render_src render/*.h render/*.c )
file( GLOB_RECURSE math_src math/*.h math/*.c )
add_library( pingo SHARED ${render_src}  ${math_src} )
target_include_directories(pingo PUBLIC
    ${CMAKE_SOURCE_DIR}/render
    ${CMAKE_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_SOURCE_DIR}
)
if(UNIX) 
  target_link_libraries(pingo m) 
endif()


# Assets library 
file( GLOB_RECURSE assets_src assets/*.h assets/*.c )
add_library( assets SHARED ${assets_src})
if (UNIX)
  target_link_libraries(assets m)
endif()

# Test suite
add_subdirectory(tests)

# Backend libraries from existing sources
if (WIN32)
  # Windows backend library
  add_library( win32_window_backend SHARED example/win32_window/windowbackend.c )
  target_link_libraries(win32_window_backend pingo gdi32)
  target_include_directories(win32_window_backend PRIVATE example/win32_window)
  
  add_executable( win_window example/common/main.c )
  target_link_libraries(win_window pingo assets win32_window_backend gdi32)
endif (WIN32)

if (UNIX)
  # Linux Window Backend (requires X11)
  add_library( linux_window_backend SHARED example/linux_window/linux_window_backend.c )
  target_link_libraries(linux_window_backend pingo X11)
  target_include_directories(linux_window_backend PRIVATE example/linux_window)
  
  add_executable( linux_window example/common/main.c )
  target_link_libraries(linux_window pingo assets linux_window_backend X11 jpeg)

  # JPEG Backend (requires libjpeg)  
  add_library( jpeg_backend SHARED example/render_to_image/jpeg_backend.c )
  target_link_libraries(jpeg_backend pingo jpeg)
  target_include_directories(jpeg_backend PRIVATE example/render_to_image)
  
  add_executable( render_to_image example/common/main.c )
  target_link_libraries(render_to_image pingo assets jpeg_backend jpeg)

  # Linux Framebuffer Backend
  add_library( linux_framebuffer_backend SHARED example/linux_framebuffer/linux_framebuffer_backend.c )
  target_link_libraries(linux_framebuffer_backend pingo)
  target_include_directories(linux_framebuffer_backend PRIVATE example/linux_framebuffer)
  
  add_executable( linux_framebuffer example/common/main.c )
  target_link_libraries(linux_framebuffer pingo assets linux_framebuffer_backend)

  # Terminal Backend
  add_library( terminal_backend SHARED example/terminal/terminalbackend.c )
  target_link_libraries(terminal_backend pingo)
  target_include_directories(terminal_backend PRIVATE example/terminal)
  
  add_executable( linux_terminal example/common/main.c )
  target_link_libraries(linux_terminal pingo assets terminal_backend)

  # Copy assets for examples that need them
  configure_file(${CMAKE_SOURCE_DIR}/assets/viking.rgba ${CMAKE_BINARY_DIR}/assets/viking.rgba COPYONLY)
endif (UNIX)

include_directories( ./ )

