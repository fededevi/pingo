# Generated mesh data tables (viking, cube, teapot, pingo).

# viking's texture coordinates are a single 60 KB array, which a 16-bit
# address space cannot represent, so there is nothing to build there.
if(CMAKE_SIZEOF_VOID_P LESS 4)
    message(STATUS "16-bit target - skipping the mesh tables")
    return()
endif()

file(GLOB_RECURSE assets_src CONFIGURE_DEPENDS *.c)
list(FILTER assets_src EXCLUDE REGEX "/tests/")

add_library(pingo_assets ${assets_src})

target_include_directories(pingo_assets PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pingo>)

# For the Mesh definition the tables are typed with.
target_link_libraries(pingo_assets PUBLIC pingo_render)

install(TARGETS pingo_assets
    EXPORT pingoTargets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo)

install(DIRECTORY ./
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo/assets
    FILES_MATCHING PATTERN "*.h")

add_subdirectory(tests)
