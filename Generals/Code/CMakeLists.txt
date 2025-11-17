# C&C Generals Code Build Configuration

# Setup include directories
set(GENERALS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake_stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/GameEngineDevice/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWLib
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWMath
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWDebug
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWSaveLoad
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WW3D2
    ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Include/Granny
)

# Add STLPort if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/STLport-4.5.3")
    list(APPEND GENERALS_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/STLport-4.5.3/stlport
    )
endif()

# Add DirectX SDK if available (Windows only)
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/DirectX")
    list(APPEND GENERALS_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/DirectX/Include
    )
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/DirectX/Lib)
endif()

include_directories(${GENERALS_INCLUDE_DIRS})

# Collect source files for WWVegas libraries
file(GLOB_RECURSE WWLIB_SOURCES
    Libraries/Source/WWVegas/WWLib/*.cpp
    Libraries/Source/WWVegas/WWLib/*.c
)

file(GLOB_RECURSE WWMATH_SOURCES
    Libraries/Source/WWVegas/WWMath/*.cpp
    Libraries/Source/WWVegas/WWMath/*.c
)

file(GLOB_RECURSE WWDEBUG_SOURCES
    Libraries/Source/WWVegas/WWDebug/*.cpp
    Libraries/Source/WWVegas/WWDebug/*.c
)

file(GLOB_RECURSE WWUTIL_SOURCES
    Libraries/Source/WWVegas/Wwutil/*.cpp
    Libraries/Source/WWVegas/Wwutil/*.c
)

file(GLOB_RECURSE WW3D2_SOURCES
    Libraries/Source/WWVegas/WW3D2/*.cpp
    Libraries/Source/WWVegas/WW3D2/*.c
)

# Conditionally exclude rendering backend files based on build options
if(USE_VULKAN)
    # Exclude DirectX 8 files when using Vulkan
    list(FILTER WW3D2_SOURCES EXCLUDE REGEX ".*dx8.*\\.(cpp|c)$")
    message(STATUS "Excluded DirectX 8 files from build (using Vulkan)")
else()
    # Exclude Vulkan files when using DirectX 8
    list(FILTER WW3D2_SOURCES EXCLUDE REGEX ".*vkwrapper.*\\.(cpp|c)$")
    list(FILTER WW3D2_SOURCES EXCLUDE REGEX ".*renderbackend.*\\.(cpp|c)$")
    message(STATUS "Excluded Vulkan files from build (using DirectX 8)")
endif()

# Collect source files for compression libraries
file(GLOB_RECURSE ZLIB_SOURCES
    Libraries/Source/Compression/ZLib/*.c
)

file(GLOB_RECURSE LZH_SOURCES
    Libraries/Source/Compression/LZHCompress/CompLibSource/*.cpp
    Libraries/Source/Compression/LZHCompress/CompLibSource/*.c
)

file(GLOB_RECURSE EAC_SOURCES
    Libraries/Source/Compression/EAC/*.cpp
    Libraries/Source/Compression/EAC/*.c
)

# Collect source files for other libraries
file(GLOB_RECURSE BENCHMARK_SOURCES
    Libraries/Source/Benchmark/*.cpp
    Libraries/Source/Benchmark/*.c
)

# Collect GameEngine source files
file(GLOB_RECURSE GAMEENGINE_SOURCES
    GameEngine/Source/*.cpp
    GameEngine/Source/*.c
)

# Collect GameEngineDevice source files
file(GLOB_RECURSE GAMEENGINEDEVICE_SOURCES
    GameEngineDevice/Source/*.cpp
    GameEngineDevice/Source/*.c
)

# Collect Main source files
file(GLOB MAIN_SOURCES
    Main/*.cpp
    Main/*.c
)

# Create WWVegas libraries
if(WWLIB_SOURCES)
    add_library(WWLib STATIC ${WWLIB_SOURCES})
    target_include_directories(WWLib PUBLIC ${GENERALS_INCLUDE_DIRS})
endif()

if(WWMATH_SOURCES)
    add_library(WWMath STATIC ${WWMATH_SOURCES})
    target_include_directories(WWMath PUBLIC ${GENERALS_INCLUDE_DIRS})
endif()

if(WWDEBUG_SOURCES)
    add_library(WWDebug STATIC ${WWDEBUG_SOURCES})
    target_include_directories(WWDebug PUBLIC ${GENERALS_INCLUDE_DIRS})
endif()

if(WWUTIL_SOURCES)
    add_library(WWUtil STATIC ${WWUTIL_SOURCES})
    target_include_directories(WWUtil PUBLIC ${GENERALS_INCLUDE_DIRS})
endif()

if(WW3D2_SOURCES)
    add_library(WW3D2 STATIC ${WW3D2_SOURCES})
    target_include_directories(WW3D2 PUBLIC ${GENERALS_INCLUDE_DIRS})

    # Add Vulkan support if enabled
    if(USE_VULKAN AND Vulkan_FOUND)
        target_include_directories(WW3D2 PUBLIC ${Vulkan_INCLUDE_DIRS})
        target_link_libraries(WW3D2 PUBLIC ${Vulkan_LIBRARIES})

        # Add shader output directory to include path
        target_include_directories(WW3D2 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/shaders)

        # Compile Vulkan shaders
        include(${CMAKE_SOURCE_DIR}/cmake/CompileShaders.cmake)

        # Find Python3 for shader header generation
        find_package(Python3 COMPONENTS Interpreter)
        if(Python3_FOUND)
            set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
            file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

            # Define shaders to compile
            set(BASIC_VERT_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Vulkan/basic.vert.glsl)
            set(BASIC_FRAG_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Vulkan/basic.frag.glsl)

            set(BASIC_VERT_HEADER ${SHADER_OUTPUT_DIR}/basic_vert.spv.h)
            set(BASIC_FRAG_HEADER ${SHADER_OUTPUT_DIR}/basic_frag.spv.h)

            # Compile shaders
            compile_shader(WW3D2 ${BASIC_VERT_SHADER} ${BASIC_VERT_HEADER} basic_vert_spv)
            compile_shader(WW3D2 ${BASIC_FRAG_SHADER} ${BASIC_FRAG_HEADER} basic_frag_spv)

            message(STATUS "Vulkan shaders will be compiled to ${SHADER_OUTPUT_DIR}")
        else()
            message(WARNING "Python3 not found - cannot generate shader headers")
        endif()
    endif()
endif()

# Create compression libraries
if(ZLIB_SOURCES)
    add_library(ZLib STATIC ${ZLIB_SOURCES})
    target_include_directories(ZLib PUBLIC
        ${GENERALS_INCLUDE_DIRS}
        Libraries/Source/Compression/ZLib
    )
endif()

if(LZH_SOURCES)
    add_library(LZHCompress STATIC ${LZH_SOURCES})
    target_include_directories(LZHCompress PUBLIC
        ${GENERALS_INCLUDE_DIRS}
        Libraries/Source/Compression/LZHCompress/CompLibHeader
    )
endif()

if(EAC_SOURCES)
    add_library(EAC STATIC ${EAC_SOURCES})
    target_include_directories(EAC PUBLIC ${GENERALS_INCLUDE_DIRS})
endif()

# Create Benchmark library
if(BENCHMARK_SOURCES)
    add_library(Benchmark STATIC ${BENCHMARK_SOURCES})
    target_include_directories(Benchmark PUBLIC ${GENERALS_INCLUDE_DIRS})
endif()

# Create GameEngine library
if(GAMEENGINE_SOURCES)
    add_library(GameEngine STATIC ${GAMEENGINE_SOURCES})
    target_include_directories(GameEngine PUBLIC ${GENERALS_INCLUDE_DIRS})

    # Link with WWVegas libraries
    if(TARGET WWLib)
        target_link_libraries(GameEngine PUBLIC WWLib)
    endif()
    if(TARGET WWMath)
        target_link_libraries(GameEngine PUBLIC WWMath)
    endif()
    if(TARGET WWDebug)
        target_link_libraries(GameEngine PUBLIC WWDebug)
    endif()
    if(TARGET WWUtil)
        target_link_libraries(GameEngine PUBLIC WWUtil)
    endif()
    if(TARGET WW3D2)
        target_link_libraries(GameEngine PUBLIC WW3D2)
    endif()
endif()

# Create GameEngineDevice library
if(GAMEENGINEDEVICE_SOURCES)
    add_library(GameEngineDevice STATIC ${GAMEENGINEDEVICE_SOURCES})
    target_include_directories(GameEngineDevice PUBLIC ${GENERALS_INCLUDE_DIRS})

    if(TARGET GameEngine)
        target_link_libraries(GameEngineDevice PUBLIC GameEngine)
    endif()
endif()

# Create main executable
if(MAIN_SOURCES)
    add_executable(Generals WIN32 ${MAIN_SOURCES})
    target_include_directories(Generals PRIVATE ${GENERALS_INCLUDE_DIRS})

    # Link all libraries
    set(GENERALS_LIBS)

    if(TARGET GameEngine)
        list(APPEND GENERALS_LIBS GameEngine)
    endif()

    if(TARGET GameEngineDevice)
        list(APPEND GENERALS_LIBS GameEngineDevice)
    endif()

    if(TARGET WWLib)
        list(APPEND GENERALS_LIBS WWLib)
    endif()

    if(TARGET WWMath)
        list(APPEND GENERALS_LIBS WWMath)
    endif()

    if(TARGET WWDebug)
        list(APPEND GENERALS_LIBS WWDebug)
    endif()

    if(TARGET WWUtil)
        list(APPEND GENERALS_LIBS WWUtil)
    endif()

    if(TARGET WW3D2)
        list(APPEND GENERALS_LIBS WW3D2)
    endif()

    if(TARGET Benchmark)
        list(APPEND GENERALS_LIBS Benchmark)
    endif()

    if(TARGET ZLib)
        list(APPEND GENERALS_LIBS ZLib)
    endif()

    if(TARGET LZHCompress)
        list(APPEND GENERALS_LIBS LZHCompress)
    endif()

    if(TARGET EAC)
        list(APPEND GENERALS_LIBS EAC)
    endif()

    target_link_libraries(Generals PRIVATE ${GENERALS_LIBS})

    # Windows-specific libraries
    if(WIN32)
        target_link_libraries(Generals PRIVATE
            kernel32
            user32
            gdi32
            winspool
            comdlg32
            advapi32
            shell32
            ole32
            oleaut32
            uuid
            odbc32
            odbccp32
            winmm
            vfw32
            dsound
            wsock32
            imm32
            wininet
        )

        # DirectX libraries (if available)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/DirectX")
            target_link_libraries(Generals PRIVATE
                dinput8
                dxguid
                d3d8
                d3dx8
            )
        endif()
    endif()

    # Set output directory
    set_target_properties(Generals PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../Run"
        OUTPUT_NAME "Generals"
    )
endif()

# Print library status
message(STATUS "Generals Libraries Configuration:")
message(STATUS "  WWLib sources: ${WWLIB_SOURCES}" )
message(STATUS "  WWMath sources: ${WWMATH_SOURCES}")
message(STATUS "  WWDebug sources: ${WWDEBUG_SOURCES}")
message(STATUS "  GameEngine sources: ${GAMEENGINE_SOURCES}")
message(STATUS "  GameEngineDevice sources: ${GAMEENGINEDEVICE_SOURCES}")
message(STATUS "  Main sources: ${MAIN_SOURCES}")
