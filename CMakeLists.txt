cmake_minimum_required(VERSION 3.15)

# Command & Conquer Generals (inc. Zero Hour) Build System
project(CnC_Generals VERSION 1.8.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_GENERALS "Build C&C Generals" ON)
option(BUILD_GENERALSZH "Build C&C Generals Zero Hour" ON)
option(BUILD_TOOLS "Build development tools" OFF)
option(PERMISSIVE_BUILD "Allow compilation to continue despite missing dependencies" ON)

# Platform checks
if(NOT WIN32 AND NOT UNIX)
    message(WARNING "This game was originally developed for Windows. Linux/Unix builds are experimental.")
endif()

# Compiler-specific settings
if(MSVC)
    # MSVC-specific flags
    add_compile_options(
        /W3                     # Warning level 3
        /permissive-            # Standards conformance
        /Zc:__cplusplus         # Enable updated __cplusplus macro
        /D_CRT_SECURE_NO_WARNINGS
        /D_SCL_SECURE_NO_WARNINGS
        /wd4068                 # Disable unknown pragma warnings (for GCC/Clang pragmas)
    )

    # Disable specific warnings that are common in old code
    add_compile_options(
        /wd4100  # Unreferenced formal parameter
        /wd4127  # Conditional expression is constant
        /wd4189  # Local variable initialized but not referenced
        /wd4201  # Nameless struct/union
        /wd4244  # Conversion from X to Y, possible loss of data
        /wd4267  # Conversion from size_t to smaller type
        /wd4305  # Truncation from double to float
        /wd4458  # Declaration hides class member
        /wd4505  # Unreferenced local function removed
        /wd4514  # Unreferenced inline function removed
        /wd4702  # Unreachable code
        /wd4706  # Assignment within conditional
        /wd4711  # Function selected for automatic inline expansion
        /wd4786  # Identifier truncated in debug info
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang-specific flags
    add_compile_options(
        -Wall
        -Wno-unknown-pragmas
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-unused-function
        -Wno-reorder
        -Wno-switch
        -Wno-parentheses
        -Wno-comment
        -Wno-format
        -Wno-sign-compare
        -Wno-narrowing
        -Wno-write-strings
        -Wno-strict-aliasing
        -fpermissive            # Allow some non-conforming code
        -Wno-deprecated
        -Wno-deprecated-declarations
    )

    # Platform-specific defines for GCC/Clang
    add_compile_definitions(
        _LINUX
        LINUX
    )
endif()

# Common definitions
add_compile_definitions(
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:_RELEASE>
    WIN32
    _WINDOWS
    _MBCS
)

# Global include directories (add paths that are needed everywhere)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Check for required dependencies
set(MISSING_DEPS "")

# DirectX SDK (Windows only)
if(WIN32)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Generals/Code/Libraries/DirectX")
        list(APPEND MISSING_DEPS "DirectX SDK (expected at Generals/Code/Libraries/DirectX)")
    endif()
endif()

# STLport
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Generals/Code/Libraries/STLport-4.5.3")
    list(APPEND MISSING_DEPS "STLport-4.5.3 (expected at Generals/Code/Libraries/STLport-4.5.3)")
endif()

# 3DS Max SDK (for tools)
if(BUILD_TOOLS AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Generals/Code/Libraries/max4sdk")
    list(APPEND MISSING_DEPS "3DS Max 4 SDK (expected at Generals/Code/Libraries/max4sdk)")
endif()

# Report missing dependencies
if(MISSING_DEPS)
    message(STATUS "=================================================================")
    message(STATUS "MISSING DEPENDENCIES DETECTED:")
    foreach(dep ${MISSING_DEPS})
        message(STATUS "  - ${dep}")
    endforeach()
    message(STATUS "")
    message(STATUS "Please refer to the README.md for information on obtaining these")
    message(STATUS "dependencies. The build will continue but may fail during linking.")
    message(STATUS "=================================================================")

    if(NOT PERMISSIVE_BUILD)
        message(FATAL_ERROR "Missing required dependencies. Set PERMISSIVE_BUILD=ON to continue anyway.")
    endif()
endif()

# Add subdirectories
if(BUILD_GENERALS)
    add_subdirectory(Generals/Code)
endif()

if(BUILD_GENERALSZH)
    # Zero Hour uses similar structure
    # add_subdirectory(GeneralsMD/Code)
endif()

# Print build configuration
message(STATUS "=================================================================")
message(STATUS "C&C Generals Build Configuration:")
message(STATUS "  CMake Version: ${CMAKE_VERSION}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Generals: ${BUILD_GENERALS}")
message(STATUS "  Build Zero Hour: ${BUILD_GENERALSZH}")
message(STATUS "  Build Tools: ${BUILD_TOOLS}")
message(STATUS "=================================================================")
