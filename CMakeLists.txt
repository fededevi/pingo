cmake_minimum_required(VERSION 3.0)
project( pingo)

# Prevent in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory.")
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
    ${CMAKE_SOURCE_DIR}
)
if(UNIX) 
  target_link_libraries(pingo m) 
endif()

# Assets library 
file( GLOB_RECURSE assets_src assets/*.h assets/*.c )
add_library( assets SHARED ${assets_src})
target_include_directories(assets PUBLIC
    ${CMAKE_SOURCE_DIR}
)
if (UNIX)
  target_link_libraries(assets m)
endif()

# Backends libraries
add_subdirectory(backends)
