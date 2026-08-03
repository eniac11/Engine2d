find_program(SpirvTools_link_PROGRAM
    NAMES spirv-link
)

if(SpirvTools_link_PROGRAM)

    add_executable(SpirvTools_link IMPORTED)
    set_target_properties(SpirvTools_link PROPERTIES
        IMPORTED_LOCATION ${SpirvTools_link_PROGRAM}
    )
    add_executable(SpirvTools::spirv_link ALIAS SpirvTools_link)
    set(SpirvTools_link_FOUND True)
else()
    set(SpirvTools_link_FOUND False)
endif()