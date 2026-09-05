include(FeatureFindPackage)

add_subdirectory(imgui)

defer_on_missing_required_packages()

set(VFSPP_VERSION 1.0)
add_library(vfspp INTERFACE)
set_target_properties(vfspp PROPERTIES HEADER_ONLY ON)
set(VFSPP_HEADERS
        vfspp/include/vfspp/Alias.hpp
        vfspp/include/vfspp/FileInfo.hpp
        vfspp/include/vfspp/FilesystemCompat.hpp
        vfspp/include/vfspp/Global.h
        vfspp/include/vfspp/IFile.h
        vfspp/include/vfspp/IFileSystem.h
        vfspp/include/vfspp/MemoryFile.hpp
        vfspp/include/vfspp/MemoryFileSystem.hpp
        vfspp/include/vfspp/NativeFile.hpp
        vfspp/include/vfspp/NativeFileSystem.hpp
        vfspp/include/vfspp/ThreadingPolicy.hpp
        vfspp/include/vfspp/VirtualFileSystem.hpp
        patches/vfspp/include/vfspp/VFS.h
)
set_property(SOURCE ${VFSPP_HEADERS} PROPERTY HEADER_FILE_ONLY ON)
target_sources(vfspp
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS vfspp/include patches/vfspp/include
        FILES
            ${VFSPP_HEADERS}
)
target_compile_features(vfspp INTERFACE cxx_std_20)
add_library(vfspp::vfspp ALIAS vfspp)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
        patches/vfspp/VFSppConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/vfspp/VFSppConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vfspp)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/vfspp/VFSppVersion.cmake
        COMPATIBILITY SameMajorVersion
        VERSION ${VFSPP_VERSION}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vfspp/VFSppConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/vfspp/VFSppVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vfspp)

install(TARGETS vfspp EXPORT VFSppTargets
    FILE_SET HEADERS
)

install(EXPORT VFSppTargets
        FILE VFSppTargets.cmake
        NAMESPACE vfspp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vfspp
)

