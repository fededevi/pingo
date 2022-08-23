cmake_minimum_required(VERSION 3.0)
project( pingo)

# Pingo rendering library
file( GLOB_RECURSE render_src render/*.h render/*.c )
file( GLOB_RECURSE math_src math/*.h math/*.c )
add_library( pingo ${render_src}  ${math_src} )
target_link_libraries(pingo m)

# Assets library 
file( GLOB_RECURSE assets_src assets/*.h assets/*.c )
add_library( assets ${assets_src})
target_link_libraries(assets pingo m)

#examples sources
file( GLOB_RECURSE linux_window_src example/linux_window/*.h example/linux_window/*.c )
file( GLOB_RECURSE linux_framebuffer_src example/linux_framebuffer/*.h example/linux_framebuffer/*.c )
file( GLOB_RECURSE terminal_src example/terminal/*.h example/terminal/*.c )
file( GLOB_RECURSE win32_window_src example/win32_window/*.h example/win32_window/*.c )

include_directories( ./ )

if (WIN32)
  add_executable( win_window ${win32_window_src} )
  target_link_libraries(win32_window pingo assets gdi32)

  add_executable( win_terminal ${terminal_src} )
  target_link_libraries(win_terminal pingo assets)
endif (WIN32)

if (UNIX)
  add_executable( linux_window ${linux_window_src} )
  target_link_libraries(linux_window pingo assets X11)

  add_executable( linux_framebuffer ${linux_framebuffer_src} )
  target_link_libraries(linux_framebuffer pingo assets)

  add_executable( linux_terminal ${terminal_src} )
  target_link_libraries(linux_terminal pingo assets)
endif (UNIX)

