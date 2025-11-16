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
option(USE_VULKAN "Use Vulkan rendering instead of DirectX 8" ON)
option(USE_DX8 "Use DirectX 8 rendering (Windows only)" OFF)

# Display enhancement options
option(ENABLE_60FPS "Enable 60 FPS support (default 30 FPS legacy)" ON)
option(ENABLE_ULTRAWIDE "Enable ultra-wide display support (21:9, 32:9)" ON)
option(ENABLE_UNCAPPED_FPS "Allow uncapped frame rate" ON)

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

# Vulkan support
if(USE_VULKAN)
    find_package(Vulkan)
    if(Vulkan_FOUND)
        message(STATUS "Vulkan found: ${Vulkan_LIBRARY}")
        add_compile_definitions(USE_VULKAN=1)
    else()
        message(WARNING "Vulkan not found. Install Vulkan SDK from https://vulkan.lunarg.com/")
        if(NOT PERMISSIVE_BUILD)
            message(FATAL_ERROR "Vulkan required but not found")
        endif()
    endif()
endif()

# Display enhancements
if(ENABLE_60FPS)
    add_compile_definitions(ENABLE_60FPS=1)
    message(STATUS "60 FPS support: ENABLED")
else()
    add_compile_definitions(ENABLE_60FPS=0)
    message(STATUS "60 FPS support: DISABLED (legacy 30 FPS)")
endif()

if(ENABLE_ULTRAWIDE)
    add_compile_definitions(ENABLE_ULTRAWIDE=1)
    message(STATUS "Ultra-wide support: ENABLED")
else()
    add_compile_definitions(ENABLE_ULTRAWIDE=0)
    message(STATUS "Ultra-wide support: DISABLED")
endif()

if(ENABLE_UNCAPPED_FPS)
    add_compile_definitions(ENABLE_UNCAPPED_FPS=1)
    message(STATUS "Uncapped FPS: ENABLED")
else()
    add_compile_definitions(ENABLE_UNCAPPED_FPS=0)
    message(STATUS "Uncapped FPS: DISABLED")
endif()

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
message(STATUS "  Rendering Backend:")
if(USE_VULKAN AND Vulkan_FOUND)
    message(STATUS "    - Vulkan: YES (${Vulkan_VERSION})")
else()
    message(STATUS "    - Vulkan: NO")
endif()
if(USE_DX8 AND WIN32)
    message(STATUS "    - DirectX 8: YES")
else()
    message(STATUS "    - DirectX 8: NO")
endif()
message(STATUS "=================================================================")
