cmake_minimum_required(VERSION 3.0)
project( pingo)

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
if(UNIX) 
  target_link_libraries(pingo m) 
endif()

# Assets library 
file( GLOB_RECURSE assets_src assets/*.h assets/*.c )
add_library( assets SHARED ${assets_src})
if (UNIX)
  target_link_libraries(assets m)
endif()

# Backends libraries
add_subdirectory(backends)

include_directories( ./ )
