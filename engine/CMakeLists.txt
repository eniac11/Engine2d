cmake_minimum_required(VERSION 3.28)
project(Engine VERSION 0.2.0)
list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(FeatureFindPackage)

# TODO: Detect whether INSTALL_PREFIX is not /opt/, /usr/, /usr/local and set ENGINE_RELOCATABLE_INSTALL
option(ENGINE_RELOCATABLE_INSTALL "Allow install_prefix to be relocatable. This means that all libraries should resolve relative to the install_prefix. This is additive" OFF)

include(GNUInstallDirs)

if (ENGINE_RELOCATABLE_INSTALL)
  set(PREFIX_PATH "${CMAKE_PREFIX_PATH}")
  set(PREFIX_LIBDIR "${CMAKE_PREFIX_PATH}/${CMAKE_INSTALL_LIBDIR}")
  cmake_path(RELATIVE_PATH PREFIX_PATH BASE_DIRECTORY "${PREFIX_LIBDIR}")
  set(PREFIX_LIBDIR_RELATIVE "${PREFIX_PATH}/${CMAKE_INSTALL_LIBDIR}")
  if (APPLE)
    set(PREFIX_LIBDIR_RPATH "@rpath/${PREFIX_LIBDIR_RELATIVE}")
  elseif(UNIX)
      set(PREFIX_LIBDIR_RPATH "\$ORIGIN/${PREFIX_LIBDIR_RELATIVE}")
  endif()
  list(PREPEND CMAKE_INSTALL_RPATH "${PREFIX_LIBDIR_RPATH}")
endif()

feature_find_package(Vulkan COMPONENTS glslc)

add_subdirectory(elog)
add_subdirectory(base)
add_subdirectory(e2d)

find_program(
  SHADER_HEADER_GEN_COMMAND
  NAMES shader_header_gen.py
  PATHS ${CMAKE_CURRENT_LIST_DIR}/cmake REQUIRED)
find_program(SPIRV_CROSS_COMMAND NAMES spirv-cross REQUIRED)
find_program(XXD_COMMAND NAMES xxd REQUIRED)

defer_on_missing_required_packages()



include(EmbedBinary)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  EngineConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Engine/EngineConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Engine/EngineVersion.cmake
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Engine/EngineConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Engine/EngineVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

# Install Embed spirv binary scripts
install(FILES ${PROJECT_SOURCE_DIR}/cmake/EmbedBinary.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

install(PROGRAMS ${PROJECT_SOURCE_DIR}/cmake/shader_header_gen.py
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

configure_package_config_file(
  EngineInTreeConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/InTree/EngineConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/InTree/EngineVersion.cmake
  COMPATIBILITY SameMajorVersion)

export(
      EXPORT VFSppTargets
      FILE ${CMAKE_CURRENT_BINARY_DIR}/InTree/VFSppTargets.cmake
      NAMESPACE vfspp::
)

export(
  EXPORT EngineElogTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/InTree/EngineElogTargets.cmake
  NAMESPACE engine::
)

export(
  EXPORT EngineBaseTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/InTree/EngineBaseTargets.cmake
  NAMESPACE engine::)

# export(
#   EXPORT glm
#   FILE ${CMAKE_CURRENT_BINARY_DIR}/InTree/glmTargets.cmake
#   NAMESPACE glm::)

export(
  EXPORT Engine2DTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/InTree/Engine2DTargets.cmake
  NAMESPACE engine::)
