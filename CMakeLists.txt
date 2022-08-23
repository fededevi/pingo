cmake_minimum_required(VERSION 3.0)
project( pingo)

#source
file( GLOB_RECURSE  render_src render/*.h render/*.cpp )
file( GLOB_RECURSE  math_src   math/*.h   math/*.cpp )

#examples
file( GLOB_RECURSE  linux_window_src   example/linux_window/*.h     example/linux_window/*.c )
file( GLOB_RECURSE  terminal_src    example/terminal/*.h         example/terminal/*.c     )
file( GLOB_RECURSE  win32_window_src        example/win32_window/*.h     example/win32_window/*.c )



if (WIN32)
  add_executable( win32_window ${win32_window_src} )
  target_link_libraries(win32_window m gdi32)

  add_executable( terminal ${terminal_src} )
  target_link_libraries(terminal m)
endif (WIN32)

if (UNIX)
  add_executable( linux_window ${linux_window_src} )
  target_link_libraries(linux_window m X11)

  add_executable( linux_terminal ${terminal_src} )
  target_link_libraries(linux_terminal m)
endif (UNIX)

