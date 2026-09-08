cmake_minimum_required(VERSION 3.16...3.31)

# LANGUAGES C: there is no C++ in this project, and without this CMake probes
# and ABI-tests a C++ compiler on every configure for nothing.
project(pingo
    VERSION 1.0.0
    DESCRIPTION "Software 3D renderer in C"
    LANGUAGES C)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory and run cmake from there.")
endif()

# Offer the standard build types in cmake-gui/ccmake without choosing one:
# an unset CMAKE_BUILD_TYPE is a legitimate request for no flags at all.
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)

# Needed early: the INSTALL_INTERFACE include paths below reference
# CMAKE_INSTALL_INCLUDEDIR.
include(GNUInstallDirs)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpointer-arith)
elseif(MSVC)
    add_compile_options(
        /W4      # match the warning level the GNU/Clang builds get
        /wd4305  # truncation from double to float
        /wd4244  # conversion possible loss of data
    )
endif()

# include(CTest) rather than a bare enable_testing(): it enables testing, and
# additionally writes DartConfiguration.tcl, which `ctest -T test` (the
# dashboard mode the VS Code CMake Tools extension uses) warns about when
# missing.
include(CTest)

# Applies to every target in every subdirectory.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Without this each subdirectory drops its output in its own build folder, so
# where a binary lands would depend on which CMakeLists declared it. Examples
# also load assets/viking.rgba relative to the working directory, which only
# works if the binaries share one.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------------
# Optional dependencies
#
# Only the example backends need these. Probing them turns a link-time
# `undefined reference to XOpenDisplay` into a configure-time message saying
# which examples are unavailable, so any environment builds whatever it can
# without being told what it has.
# ---------------------------------------------------------------------------
if(UNIX)
    # Not on Apple: macOS runners report X11 as found while shipping no
    # X11/Xlib.h, and the backend that needs it is Linux/X11 specific anyway.
    if(NOT APPLE)
        find_package(X11 QUIET)
    endif()
    find_package(JPEG QUIET)
endif()

# ---------------------------------------------------------------------------
# The parts. Each directory owns its library, its install rules and its tests.
# Order matters: render uses math, assets uses the Mesh type from render.
# ---------------------------------------------------------------------------
add_subdirectory(math)
add_subdirectory(render)
add_subdirectory(assets)
add_subdirectory(example)

# ---------------------------------------------------------------------------
# Package config, so the libraries are consumable via find_package(pingo).
# Each subdirectory has already added its own target to pingoTargets.
# ---------------------------------------------------------------------------
include(CMakePackageConfigHelpers)

install(EXPORT pingoTargets
    FILE pingoTargets.cmake
    NAMESPACE pingo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pingo)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pingoConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pingoConfig.cmake
"include(\"\${CMAKE_CURRENT_LIST_DIR}/pingoTargets.cmake\")\n")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pingoConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/pingoConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pingo)
