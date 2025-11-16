# CMake module for compiling GLSL shaders to SPIR-V and generating C headers

# Find shader compiler
if(Vulkan_FOUND)
    # Try to find glslc (preferred, part of Vulkan SDK)
    find_program(GLSLC_EXECUTABLE
        NAMES glslc
        HINTS
            ${Vulkan_INCLUDE_DIR}/../bin
            ${Vulkan_INCLUDE_DIR}/../Bin
            $ENV{VULKAN_SDK}/bin
            $ENV{VULKAN_SDK}/Bin
    )

    # Fallback to glslangValidator
    if(NOT GLSLC_EXECUTABLE)
        find_program(GLSLANG_VALIDATOR
            NAMES glslangValidator
            HINTS
                ${Vulkan_INCLUDE_DIR}/../bin
                ${Vulkan_INCLUDE_DIR}/../Bin
                $ENV{VULKAN_SDK}/bin
                $ENV{VULKAN_SDK}/Bin
        )
    endif()

    if(GLSLC_EXECUTABLE)
        message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")
        set(SHADER_COMPILER ${GLSLC_EXECUTABLE})
        set(SHADER_COMPILER_FLAGS "-c")
    elseif(GLSLANG_VALIDATOR)
        message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")
        set(SHADER_COMPILER ${GLSLANG_VALIDATOR})
        set(SHADER_COMPILER_FLAGS "-V")
    else()
        message(WARNING "No SPIR-V compiler found. Shaders will not be compiled.")
        set(SHADER_COMPILER "")
    endif()
endif()

# Function to compile a single GLSL shader to SPIR-V and generate C header
function(compile_shader TARGET_NAME SHADER_SOURCE OUTPUT_HEADER ARRAY_NAME)
    if(NOT SHADER_COMPILER)
        message(WARNING "Cannot compile shader ${SHADER_SOURCE}: no compiler found")
        return()
    endif()

    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
    set(SPIRV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")

    # Command to compile GLSL to SPIR-V
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${SHADER_COMPILER} ${SHADER_COMPILER_FLAGS}
                ${SHADER_SOURCE} -o ${SPIRV_OUTPUT}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling shader ${SHADER_NAME} to SPIR-V"
        VERBATIM
    )

    # Command to convert SPIR-V to C header
    add_custom_command(
        OUTPUT ${OUTPUT_HEADER}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/spirv_to_header.py
                ${SPIRV_OUTPUT} ${OUTPUT_HEADER} ${ARRAY_NAME}
        DEPENDS ${SPIRV_OUTPUT} ${CMAKE_SOURCE_DIR}/spirv_to_header.py
        COMMENT "Generating C header ${OUTPUT_HEADER}"
        VERBATIM
    )

    # Add to target dependencies
    set_source_files_properties(${OUTPUT_HEADER} PROPERTIES GENERATED TRUE)
endfunction()

# Function to compile multiple shaders
function(compile_shaders TARGET_NAME)
    set(SHADER_HEADERS "")

    foreach(SHADER_INFO ${ARGN})
        # Parse shader info: "source;header;array_name"
        string(REPLACE ";" " " SHADER_INFO_STR "${SHADER_INFO}")
        separate_arguments(SHADER_PARTS UNIX_COMMAND "${SHADER_INFO_STR}")
        list(GET SHADER_PARTS 0 SHADER_SOURCE)
        list(GET SHADER_PARTS 1 OUTPUT_HEADER)
        list(GET SHADER_PARTS 2 ARRAY_NAME)

        compile_shader(${TARGET_NAME} ${SHADER_SOURCE} ${OUTPUT_HEADER} ${ARRAY_NAME})
        list(APPEND SHADER_HEADERS ${OUTPUT_HEADER})
    endforeach()

    # Create a custom target that depends on all shader headers
    add_custom_target(${TARGET_NAME}_shaders ALL DEPENDS ${SHADER_HEADERS})
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
endfunction()
