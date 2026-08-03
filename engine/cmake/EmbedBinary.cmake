include_guard(GLOBAL)
if (DEFINED SPIRV_COMPILE_INCLUDES)
    list(TRANSFORM GLSLC_INCLUDE_DIRS PREPEND -I)
endif()

function(custom_shader_compile TARGET STAGE OUT_VAR)
    string(TOLOWER ${STAGE} STAGE_lowered)
    set(INPUT_FILES ${ARGN})

    foreach(INPUT_FILE "${INPUT_FILES}")
        cmake_path(GET INPUT_FILE PARENT_PATH DIR_PART)
        cmake_path(GET INPUT_FILE STEM NAME_PART)
        cmake_path(GET INPUT_FILE EXTENSION EXT_PART)
        string(JOIN "" output_file ${TARGET}__ ${NAME_PART} ".spv.o")
        string(JOIN "/" output_file_path ${DIR_PART} ${output_file})
        list(APPEND OUTPUT_FILES ${output_file_path})

    endforeach()
    list(TRANSFORM INPUT_FILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
    set(TARGET_NAME ${TARGET}__${STAGE_lowered})
    set(OUTPUT_FILE_NAME ${TARGET_NAME}.spv.o)
    add_custom_target(${TARGET_NAME} DEPENDS ${OUTPUT_FILE_NAME})
    add_custom_command(
        OUTPUT ${OUTPUT_FILE_NAME}
        COMMAND Vulkan::glslc
        ARGS
        --target-env=opengl
#        -fauto-bind-uniforms
#        -fauto-map-locations
        $<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>,PREPEND,-I>
        -MD
        -fshader-stage=${STAGE_lowered}
        -o ${OUTPUT_FILE_NAME}
        ${INPUT_FILES}
        DEPENDS
        ${INPUT_FILES}
        BYPRODUCTS
        ${OUTPUT_FILE_NAME}.d
        DEPFILE
        ${OUTPUT_FILE_NAME}.d
        COMMENT "Building ${OUTPUT_FILE_NAME}"
        COMMAND_EXPAND_LISTS
    )


    set(${OUT_VAR} ${OUTPUT_FILE_NAME} PARENT_SCOPE)

endfunction()

function(add_spirv_shader TARGET)

    set(MULTI_VALUE "VERTEX;FRAGMENT;GEOMETRY;TESS;COMPUTE")
    set(options )
    set(oneValueArgs )
    set(multiValueArgs VERTEX FRAGMENT GEOMETRY TESS COMPUTE)
    cmake_parse_arguments(AddSS
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
        ${ARGN}
    )

    add_library(${TARGET} OBJECT)


    if(AddSS_VERTEX)
        custom_shader_compile(${TARGET} VERTEX vertex_o_files ${AddSS_VERTEX})
        list(APPEND targets ${TARGET}__vertex)
        list(APPEND input_files  ${vertex_o_files})
    endif()
    if(AddSS_FRAGMENT)
        custom_shader_compile(${TARGET} FRAGMENT fragment_o_files ${AddSS_FRAGMENT})
        list(APPEND targets ${TARGET}__fragment)
        list(APPEND input_files  ${fragment_o_files})
    endif()
    if(AddSS_COMPUTE)
        custom_shader_compile(${TARGET} COMPUTE compute_o_files ${AddSS_COMPUTE})
        list(APPEND targets ${TARGET}__compute)
        list(APPEND input_files  ${compute_o_files})
    endif()
    set(target_file ${TARGET}.spv)
    add_custom_command(
        OUTPUT ${target_file}
        COMMAND SpirvTools::spirv_link
        ARGS
        --target-env opengl4.5
        -o ${target_file}
        ${input_files}
        DEPENDS
        ${input_files}
        ${targets}
        COMMENT "Linking ${target_file}"

    )

    target_sources(${TARGET} PRIVATE ${target_file})
    add_dependencies(${TARGET} ${targets})
endfunction()

function(generate_shader_reflection_header TARGET)
    set(json_file ${TARGET}.spv.json)
    set(c_file ${TARGET}.spv.c)
    set(header_file ${TARGET}.h)
    set(extern_name ${TARGET}_spv)
    set(spv_file ${TARGET}.spv)
    add_custom_command(
        OUTPUT ${c_file} ${header_file}
        COMMAND ${SPIRV_CROSS_COMMAND} ARGS ${TARGET}.spv --reflect --output ${json_file}
        COMMAND ${SHADER_HEADER_GEN_COMMAND} ARGS ${json_file} ${TARGET} ${extern_name} ${header_file}
        COMMAND ${XXD_COMMAND} ARGS -n ${extern_name} -i ${spv_file} ${c_file}
        DEPENDS
        ${spv_file}
        ${TARGET}
        ${SHADER_HEADER_GEN_COMMAND}
        BYPRODUCTS
        ${json_file}
    )
    target_sources(${TARGET} PUBLIC FILE_SET public_headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR} FILES ${CMAKE_CURRENT_BINARY_DIR}/${header_file} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/${c_file})
#    add_dependencies(${TARGET} ${TARGET}_generated)
#    set(${GENERATED_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${c_file} ${header_file} PARENT_SCOPE)
endfunction()
