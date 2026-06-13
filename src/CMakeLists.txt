#-------------------------------------------------------------------------------
# Global settings
#-------------------------------------------------------------------------------

message(STATUS "CMake version is ${CMAKE_VERSION}")

# 3.11 required for find_package(ICU) with COMPONENTS and imported targets
# (FindICU was added in 3.7; imported targets ICU::uc arrived in 3.11)
# 3.12 required to avoid warning
# Policy CMP0074 is not set: find_package uses <PackageName>_ROOT variables.
cmake_minimum_required(VERSION 3.12...99.99)

# must come before the project command
# 10.13.6 High Sierra is the minimum system supported
if( APPLE AND BUILD_OSX_UNIVERSAL_BINARIES )
  set( CMAKE_OSX_DEPLOYMENT_TARGET 10.13.6 CACHE STRING  "" FORCE)
endif()

project(ICU4ooRexx)

if( APPLE AND BUILD_OSX_UNIVERSAL_BINARIES )
  set( CMAKE_OSX_ARCHITECTURES arm64 x86_64 )
endif()

# Needed for ooRexx 4.2 when compiling with Microsoft cl
include(CheckIncludeFile)
check_include_file(stdint.h HAVE_STDINT_H)
configure_file(config.h.in.cmake ${PROJECT_BINARY_DIR}/config.h)


#-------------------------------------------------------------------------------
# C++ standard
#-------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)  # Starting with ICU 75 (April 2024), ICU4C requires C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#-------------------------------------------------------------------------------
# Find ICU4C
#
# CMake's built-in FindICU module (since 3.7) looks in standard system
# locations and respects:
#   -DICU_ROOT=<path>            cmake command-line hint
#   ICU_ROOT environment variable
#
# Typical non-standard locations:
#   macOS Homebrew (Apple Silicon):  ICU_ROOT=/opt/homebrew/opt/icu4c
#   macOS Homebrew (Intel):          ICU_ROOT=/usr/local/opt/icu4c
#   Windows custom install:          ICU_ROOT=C:/icu
#-------------------------------------------------------------------------------

# https://cmake.org/cmake/help/latest/module/FindICU.html

# Test in priority homebrew, because the Xcode's version of ICU is always older.
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/icu4c")
        list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/icu4c")
        find_package(ICU 70 COMPONENTS dt i18n tu uc)
    endif()
endif()

if(NOT ICU_FOUND)
    find_package(ICU 70 COMPONENTS dt i18n tu uc)
endif()

if(NOT ICU_FOUND AND ICU_ROOT)
    list(PREPEND CMAKE_PREFIX_PATH "${ICU_ROOT}")
    find_package(ICU 70 COMPONENTS dt i18n tu uc)
endif()

if(NOT ICU_FOUND)
    message(FATAL_ERROR "ICU not found. Set ICU_ROOT or install ICU.")
endif()

message(STATUS "ICU found: ${ICU_FOUND}")
message(STATUS "ICU version: ${ICU_VERSION}")
message(STATUS "ICU include dirs:  ${ICU_INCLUDE_DIRS}")
message(STATUS "ICU libraries: ${ICU_LIBRARIES}")

# ICU datafiles
message(STATUS "ICU path to Makefile.inc: ${ICU_MAKEFILE_INC}")
message(STATUS "ICU path to pkgdata.inc: ${ICU_PKGDATA_INC}")

message(STATUS "ICU icuinfo executable: ${ICU_ICUINFO_EXECUTABLE}")
message(STATUS "ICU icu-config executable: ${ICU_ICU-CONFIG_EXECUTABLE}")
message(STATUS "ICU uconv executable: ${ICU_UCONV_EXECUTABLE}")


#-------------------------------------------------------------------------------
# Find ooRexx headers and libraries
#
# We do NOT use find_library / find_path here.  Instead we rely on the
# standard compiler/linker environment variables that the user has set:
#
#   GCC / Clang:
#     CPLUS_INCLUDE_PATH   -- finds oorexxapi.h
#     LIBRARY_PATH         -- finds librexx, librexxapi
#
#   MSVC:
#     INCLUDE              -- finds oorexxapi.h
#     LIB                  -- finds rexx.lib, rexxapi.lib
#
# If you prefer explicit cmake flags, you can still pass:
#   -DCMAKE_INCLUDE_PATH=<oorexx-api-dir>
#   -DCMAKE_LIBRARY_PATH=<oorexx-lib-dir>
# which cmake merges into its own search paths.
#-------------------------------------------------------------------------------

find_path(OOREXX_INCLUDE_DIR
    NAMES oorexxapi.h
    # cmake automatically searches CMAKE_INCLUDE_PATH and the compiler
    # environment include variables (CPLUS_INCLUDE_PATH / INCLUDE)
    DOC "Directory containing oorexxapi.h"
)
if(NOT OOREXX_INCLUDE_DIR)
    message(FATAL_ERROR
        "oorexxapi.h not found.\n"
        "Set CPLUS_INCLUDE_PATH (gcc/clang) or INCLUDE (MSVC) to the "
        "ooRexx API directory, e.g.:\n"
        "  export CPLUS_INCLUDE_PATH=/usr/include/oorexx\n"
        "Or pass -DCMAKE_INCLUDE_PATH=<path> to cmake.")
endif()
message(STATUS "ooRexx include dir: ${OOREXX_INCLUDE_DIR}")

find_library(OOREXX_LIB     NAMES rexx     DOC "ooRexx runtime library")
find_library(OOREXX_API_LIB NAMES rexxapi  DOC "ooRexx API library")

if(NOT OOREXX_LIB OR NOT OOREXX_API_LIB)
    message(FATAL_ERROR
        "librexx / librexxapi (or rexx.lib / rexxapi.lib) not found.\n"
        "Set LIBRARY_PATH (gcc/clang) or LIB (MSVC) to the ooRexx lib "
        "directory, e.g.:\n"
        "  export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu\n"
        "Or pass -DCMAKE_LIBRARY_PATH=<path> to cmake.")
endif()
message(STATUS "ooRexx lib:     ${OOREXX_LIB}")
message(STATUS "ooRexx api lib: ${OOREXX_API_LIB}")


#-------------------------------------------------------------------------------
# Build the shared library
#-------------------------------------------------------------------------------

add_library(ICU4ooRexx SHARED
    ICU4ooRexx.cpp
)

target_include_directories(ICU4ooRexx PRIVATE
    ${PROJECT_BINARY_DIR}   # for config.h
    ${OOREXX_INCLUDE_DIR}
    # ICU::uc imported target already carries its include dirs, but we add
    # ICU_INCLUDE_DIRS explicitly too for compilers that need it plainly.
    ${ICU_INCLUDE_DIRS}
)

target_link_libraries(ICU4ooRexx
    ICU::uc          # ICU Common library (imported target from FindICU)
    ${OOREXX_LIB}
    ${OOREXX_API_LIB}
)


#-------------------------------------------------------------------------------
# Output naming convention:
#   Linux   -> libICU4ooRexx.so
#   macOS   -> libICU4ooRexx.dylib
#   Windows -> ICU4ooRexx.dll
#-------------------------------------------------------------------------------

if(WIN32)
    set_target_properties(ICU4ooRexx PROPERTIES
        OUTPUT_NAME "ICU4ooRexx"
        PREFIX ""
    )
    # Windows needs an export definition file so that ooRexx can find
    # RexxGetPackage without C++ name mangling.
    target_sources(ICU4ooRexx PRIVATE ICU4ooRexx.def)
else()
    set_target_properties(ICU4ooRexx PROPERTIES
        OUTPUT_NAME "ICU4ooRexx"
        PREFIX "lib"
    )
endif()


#-------------------------------------------------------------------------------
# Install
#-------------------------------------------------------------------------------

if (WIN32)
    install(TARGETS ICU4ooRexx
        LIBRARY DESTINATION .
        RUNTIME DESTINATION .
    )
    install(FILES ICU4ooRexx.cls DESTINATION .)
else ()
    install(TARGETS ICU4ooRexx
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
    install(FILES ICU4ooRexx.cls DESTINATION bin)
endif ()
