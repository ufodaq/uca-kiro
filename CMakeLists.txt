cmake_minimum_required(VERSION 2.6)
project(KIROCS C)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

set(TARNAME "kiro-camera-server")
set(LIBKIROCS_VERSION_MAJOR "0")
set(LIBKIROCS_VERSION_MINOR "0")
set(LIBKIROCS_VERSION_PATCH "1")
set(LIBKIROCS_VERSION_RELEASE "0")
set(LIBKIROCS_VERSION_STRING "${LIBKIROCS_VERSION_MAJOR}.${LIBKIROCS_VERSION_MINOR}.${LIBKIROCS_VERSION_PATCH}")
set(VERSION "${LIBKIROCS_VERSION_STRING}")
set(LIBKIROCS_DESCRIPTION "Small InfiniBand communication Server and Client")

# Increase the ABI version when binary compatibility cannot be guaranteed, e.g.
# symbols have been removed, function signatures, structures, constants etc.
# changed.
set(LIBKIROCS_ABI_VERSION "1")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/common/cmake")

include(ConfigurePaths)
include(PkgConfigVars)
configure_paths(KIROCS)

find_package(PkgConfig)
pkg_check_modules(GLIB2 glib-2.0>=2.32 REQUIRED)
pkg_check_modules(GOBJECT2 gobject-2.0>=2.32 REQUIRED)
pkg_check_modules(GMODULE2 gmodule-2.0>=2.32 REQUIRED)
pkg_check_modules(GIO2 gio-2.0>=2.32 REQUIRED)
pkg_check_modules(LIBUCA libuca>=2.0 REQUIRED)
pkg_check_modules(KIRO kiro>=1.4 REQUIRED)

include_directories(
    SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLIB2_INCLUDE_DIRS}
    ${GOBJECT2_INCLUDE_DIRS}
    ${GMODULE2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${LIBUCA_INCLUDE_DIR}
    ${KIRO_INCLUDE_DIR})

link_directories(
    ${GLIB2_LIBDIR}
    ${GOBJECT2_LIBDIR}
    ${GMODULE2_LIBDIR}
    ${GIO2_LIBDIR}
    ${LIBUCA_LIBDIR}
    ${KIRO_LIBDIR})

set(KIROCS_DEPS
    ${GLIB2_LIBRARIES}
    ${GOBJECT2_LIBRARIES}
    ${GMODULE2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${LIBUCA_LIBRARIES}
    ${KIRO_LIBRARIES})

set(LIBKIROCS_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra -std=c99)

add_subdirectory(bin)
