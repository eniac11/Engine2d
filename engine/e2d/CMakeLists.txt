include(FetchContent)
include(FeatureSummary)

fetchcontent_declare(ARGPARSE
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
        GIT_TAG v3.1
        FIND_PACKAGE_ARGS 3.1
)

FetchContent_Declare(
        beman_indirect
        GIT_REPOSITORY https://github.com/bemanproject/indirect.git
        GIT_TAG 36579dd72730156afed6247abf24835a222d7368
        FIND_PACKAGE_ARGS 0.1.0 NAMES beman.indirect
)


fetchcontent_declare(
        ldtk_parser
        GIT_REPOSITORY https://github.com/eniac11/ldtk_parser.git
        FIND_PACKAGE_ARGS 0.1 NAMES ldtk_parser
)


feature_find_package(Vulkan RECOMMENDED)
feature_find_package(OpenImageIO 2.4 RECOMMENDED)
feature_find_package(Ktx RECOMMENDED)
fetchcontent_makeavailable(ARGPARSE)
#fetchcontent_makeavailable(LDTKLOADER)
# set_package_properties(LDTKLOADER PROPERTIES TYPE REQUIRED)
#feature_find_package(ARGPARSE RUNTIME)
fetchcontent_makeavailable(ldtk_parser beman_indirect)
feature_find_package(Backward)
#set_package_properties(ARGPARSE PROPERTIES TYPE RUNTIME)

# add_subdirectory(vendor/LDtkLoader)


defer_on_missing_required_packages()
include(EmbedBinary)

add_executable(e2d_generate_texture_array_atlas2)
target_sources(e2d_generate_texture_array_atlas2
        PRIVATE
        tools/generate_texture_array_atlas2.cpp
)
target_link_libraries(e2d_generate_texture_array_atlas2 PRIVATE
        OpenImageIO::OpenImageIO
        KTX::ktx
        Vulkan::Headers
        argparse::argparse
        Backward::Backward
)
set_target_properties(e2d_generate_texture_array_atlas2 PROPERTIES CXX_STANDARD 23)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(e2d_generate_texture_array_atlas2 PRIVATE -stdlib=libstdc++)
    target_link_options(e2d_generate_texture_array_atlas2 PRIVATE -stdlib=libstdc++)
endif ()

add_spirv_shader(tilelayer_shader VERTEX data/tilelayer.vs.glsl FRAGMENT data/tilelayer.fs.glsl)
target_include_directories(tilelayer_shader PRIVATE data)
generate_shader_reflection_header(tilelayer_shader)

add_spirv_shader(sprite_shader VERTEX data/Sprite.vs.glsl FRAGMENT data/Sprite.fs.glsl)
target_include_directories(sprite_shader PRIVATE data)
generate_shader_reflection_header(sprite_shader)

add_library(e2d)
target_sources(e2d
        PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES
            include/engine/2d/TileSet.h
            include/engine/2d/tileset_mesher.h
            include/engine/2d/Chunk.h
            include/engine/2d/Sprite.h
            include/engine/2d/sprite_renderer.h
        PRIVATE
        FILE_SET private_headers
        TYPE HEADERS
        BASE_DIRS private
        FILES
        private/e2d_logging.h
        PRIVATE
            src/TileSet.cpp
            src/tileset_mesher.cpp
            src/Chunk.cpp
            src/Sprite.cpp
            src/sprite_renderer.cpp

)
target_link_libraries(e2d PUBLIC engine::engine ldtk_parser ldtk_parser_io PRIVATE  tilelayer_shader sprite_shader)

add_library(engine::e2d ALIAS e2d)
add_library(engine::tilelayer_shader ALIAS tilelayer_shader)
add_library(engine::sprite_shader ALIAS sprite_shader)
add_executable(engine::e2d_generate_texture_array_atlas2 ALIAS e2d_generate_texture_array_atlas2)

add_feature_info(IMAGE_ATLAS_GENERATOR
        "TARGET OpenImageIO::OpenImageIO AND TARGET e2d_generate_texture_array_atlas"
        "Image atlas array texture generation from texture atlases"
)

include(GNUInstallDirs)


install(TARGETS e2d tilelayer_shader sprite_shader EXPORT Engine2DTargets
        LIBRARY
        INCLUDES
        FILE_SET public_headers
)

install(TARGETS e2d_generate_texture_array_atlas2 EXPORT Engine2DTargets
        RUNTIME
)

install(EXPORT Engine2DTargets
        FILE Engine2DTargets.cmake
        NAMESPACE engine::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Engine
)
