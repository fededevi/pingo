# The maths: vectors and matrices. Depends on nothing but libm.

file(GLOB_RECURSE math_src CONFIGURE_DEPENDS *.c)
list(FILTER math_src EXCLUDE REGEX "/tests/")

add_library(pingo_math ${math_src})

target_include_directories(pingo_math PUBLIC
    # Sources refer to each other both as "vec3.h" and as "math/vec3.h".
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pingo>)

set_target_properties(pingo_math PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

if(UNIX)
    target_link_libraries(pingo_math PUBLIC m)
endif()

install(TARGETS pingo_math
    EXPORT pingoTargets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo)

install(DIRECTORY ./
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pingo/math
    FILES_MATCHING PATTERN "*.h")

add_subdirectory(tests)
