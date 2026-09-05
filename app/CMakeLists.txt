cmake_minimum_required(VERSION 3.28)
project(App VERSION 0.1.0)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/engine/InTree)
include(../cmake/FeatureFindPackage.cmake)

# TODO: Detect whether INSTALL_PREFIX is not /opt/, /usr/, /usr/local and set ENGINE_RELOCATABLE_INSTALL
option(APP_RELOCATABLE_INSTALL "Allow install_prefix to be relocatable. This means that all libraries should resolve relative to the install_prefix. This is additive" OFF)

include(GNUInstallDirs)

if (APP_RELOCATABLE_INSTALL)
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


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)

feature_find_package(Backward CONFIG REQUIRED)
feature_find_package(glfw3 3.3 REQUIRED)
feature_find_package(Engine 0.1 REQUIRED COMPONENTS Elog Base 2D)



#feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
defer_on_missing_required_packages()

add_spirv_shader(sprite_render_shader VERTEX data/sprite_render.vs.glsl FRAGMENT data/sprite_render.fs.glsl)
target_include_directories(sprite_render_shader PRIVATE data)
generate_shader_reflection_header(sprite_render_shader)

add_spirv_shader(sprite_render_shader2 VERTEX data/sprite_render2.vs.glsl FRAGMENT data/sprite_render2.fs.glsl)
target_include_directories(sprite_render_shader2 PRIVATE data)
generate_shader_reflection_header(sprite_render_shader2)

add_spirv_shader(compute_shader COMPUTE data/compute_test.comp.glsl)
target_include_directories(compute_shader PRIVATE data)
generate_shader_reflection_header(compute_shader)


add_executable(game_app)
target_sources(game_app
    PRIVATE
      src/main.cpp
      src/mainloop.cpp
      src/mainloop.h
      src/tilemap_parse_temp.cpp
      src/tilemap_parse_temp.h
      src/imgui_impl_glfw.cpp
      src/imgui_impl_glfw.h
)
target_link_libraries(game_app PRIVATE Backward::Backward glfw engine::engine engine::e2d sprite_render_shader sprite_render_shader2 engine_graphics_opengl imgui::imgui)


install(TARGETS game_app sprite_render_shader sprite_render_shader2
    RUNTIME
)
