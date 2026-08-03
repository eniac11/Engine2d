cmake_minimum_required(VERSION 3.28)

project(openglglew)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(PREPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/cmake")


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
cmake_policy(SET CMP0155 OLD)
unset(CMAKE_CXX_SCANDEP_SOURCE)
unset(CMAKE_CXX_MODULE_MAP_FORMAT)
unset(CMAKE_CXX_MODULE_MAP_FLAG)
unset(CMAKE_CXX_MODULE_BMI_ONLY_FLAG)


if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
add_compile_options(-stdlib=libc++ -fexperimental-library)
add_link_options(-stdlib=libc++ -fexperimental-library)
endif()

include(CTest)
include(FeatureSummary)
include(FeatureFindPackage)


feature_find_package(SpirvTools COMPONENTS link)


set(GLM_ENABLE_LANG_EXTENSIONS ON)
set(GLM_ENABLE_SIMD_SSE2 ON)

# feature_find_package(glm 1.1.0 REQUIRED)
feature_find_package(GTest 1.14.0 REQUIRED)

add_subdirectory(engine)
add_subdirectory(app)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

