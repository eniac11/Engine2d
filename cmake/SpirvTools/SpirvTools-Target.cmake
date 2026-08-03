find_library(SpirvTools_LIBRARY
    NAMES
    SPIRV-Tools
    HINTS ${CMAKE_SYSTEM_LIBRARY_PATH}
)
find_path(SpirvTools_INCLUDE_DIR
    NAMES libspirv.h
    PATH_SUFFIXES spirv-tools
)

add_library(SpirvTools STATIC IMPORTED)
set_target_properties(SpirvTools PROPERTIES
    IMPORTED_LOCATION ${SpirvTools_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${SpirvTools_INCLUDE_DIR}
)
add_library(SpirvTools::SpirvTools ALIAS SpirvTools)
#find_package_message()