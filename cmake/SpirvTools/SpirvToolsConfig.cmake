include(${CMAKE_CURRENT_LIST_DIR}/SpirvTools-Target.cmake)
set(_SpirvTools_supported_components diff link opt reduce)
foreach(_comp ${SpirvTools_FIND_COMPONENTS})
    if (NOT ";${_SpirvTools_supported_components};" MATCHES ";${_comp};")
        set(SpirvTools_FOUND False)
        set(SpirvTools_NOT_FOUND_MESSAGE "Unsupported Component: ${_comp}")
    endif()
    include("${CMAKE_CURRENT_LIST_DIR}/SpirvTools${_comp}-Targets.cmake")
endforeach()
if (TARGET SpirvTools::SpirvTools)
    set(SpirvTools_LIBRARIES SpirvTools::SpirvTools)
    get_target_property(SpirvTools_INCLUDE_DIRS SpirvTools::SpirvTools INTERFACE_INCLUDE_DIRECTORIES)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SpirvTools
    REQUIRED_VARS SpirvTools_LIBRARIES SpirvTools_INCLUDE_DIR
    HANDLE_COMPONENTS
)