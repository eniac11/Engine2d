include(FeatureFindPackage)

feature_find_package(OpenGL REQUIRED)
feature_find_package(GLEW REQUIRED)
feature_find_package(glm 1.1.0 REQUIRED)
feature_find_package(Ktx 4.4 REQUIRED CONFIG)
feature_find_package(dwarfs 0.15 REQUIRED)
feature_find_package(GTest 1.14.0 REQUIRED)

add_subdirectory(vendor)

defer_on_missing_required_packages()

set_property(SOURCE include/engine/graphics/backend.h PROPERTY LANGUAGE CXX)
set_property(SOURCE include/engine/uniforms.h PROPERTY LANGUAGE CXX)

add_library(engine)
target_sources(engine
        PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES
            include/engine/ShaderResources.h
            include/engine/uniforms.h
            include/engine/Camera.h
            include/engine/ShaderProgram.h
            include/engine/graphics/Buffer.h
            include/engine/graphics/Texture.h
            include/engine/graphics/VertexArray.h
            include/engine/graphics/ubo.h
            include/engine/app.h
            # include/engine/rendering/light.h
            # include/engine/rendering/Material.h
            # include/engine/models/Mesh.h
            # include/engine/models/Model.h
            include/engine/RAII/binding.h
            include/engine/uuid.h
            include/engine/resources/texturestore.h
            include/engine/resources/sha1.h
            include/engine/resources/dwarfs_fs.h
            include/engine/StandardDirectories.h
            include/engine/graphics/backend.h
            include/engine/math.h
        PRIVATE
        FILE_SET private_headers
        BASE_DIRS src/private
        TYPE HEADERS
        FILES
            src/private/engine_p/logging_categories.h
        PRIVATE
            src/app.cpp
            src/Texture.cpp
            src/Camera.cpp
            src/ShaderProgram.cpp
            src/uuid.cpp
            # src/models/Mesh.cpp
            # src/models/Model.cpp
            src/resources/texturestore.cpp
            src/resources/sha1.cpp
            src/resources/dwarfs_fs.cpp
)

if (UNIX)
    target_sources(engine PRIVATE src/StandardDirectories_linux.cpp)
endif ()
target_compile_definitions(engine PUBLIC GLM_FORCE_DEFAULT_ALIGNED_GENTYPES _MSC_EXTENSIONS GLM_ENABLE_EXPERIMENTAL)
target_compile_options(engine PUBLIC -msse2 -mavx2 -msse4.2 -fms-extensions -msha)
target_link_libraries(engine
        PUBLIC GLEW::GLEW KTX::ktx glm::glm OpenGL::OpenGL engine_elog vfspp::vfspp
        PRIVATE dwarfs::dwarfs_reader
)

add_subdirectory(backends/opengl)

defer_on_missing_required_packages()

include(GoogleTest)

add_executable(engine_tests_suite)
if (UNIX)
    target_sources(engine_tests_suite PRIVATE tests/test_standard_directories_linux.cpp)
endif ()
target_link_libraries(engine_tests_suite PRIVATE GTest::gtest_main engine)

gtest_discover_tests(engine_tests_suite)

add_library(engine::engine ALIAS engine)

include(GNUInstallDirs)


install(TARGETS engine EXPORT EngineBaseTargets
        LIBRARY
        FILE_SET public_headers

)

install(EXPORT EngineBaseTargets
        FILE EngineBaseTargets.cmake
        NAMESPACE engine::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine
)
