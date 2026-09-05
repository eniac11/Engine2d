#include "mainloop.h"

#include <vfspp/NativeFileSystem.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <engine/Camera.h>
#include <engine/2d/TileSet.h>
#include <engine/2d/tileset_mesher.h>
#include <engine/RAII/binding.h>
#include <engine/resources/dwarfs_fs.h>
#include <engine/elog.h>
#include <engine/2d/Chunk.h>
// #include <engine/imgui.h>

#include <iostream>
#include <array>
#include <string_view>
#include <memory>
#include <print>
#include <numeric>
#include <generator>
#include <chrono>


#include "sprite_render_shader2.h"
#include "tilemap_parse_temp.h"
#include "imgui/imgui.h"


ELOG_DECLARE_LOGGING_CATEGORY(lcGameApp, "game_app")

inline constexpr auto static_mesh = TileSetMesher<16, 16>::generate_mesh();


void set_window_hints() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

bool check_extensions() {
    // if ( !GLEW_ARB_bindless_texture ){
    //     // glGetTextureHandleARB()
    //     std::println("GLEW_ARB_bindless_texture NOT supported");
    //     return false;
    // }

    return true;
}


// glm::mat4 DrawSprite(glm::vec2 position,
//   glm::vec2 size, float rotate, glm::vec3 color) {
//     glm::mat4 model = glm::mat4(1.0f);
//     model = glm::translate(model, glm::vec3(position, 0.0f));
//
//     model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y,
//     0.0f)); model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f,
//     0.0f, 1.0f)); model = glm::translate(model, glm::vec3(-0.5f * size.x,
//     -0.5f * size.y, 0.0f));
//
//     model = glm::scale(model, glm::vec3(size, 1.0f));
//     return model;

// }

std::vector<std::array<std::uint32_t, 16*16>> tiledata{
                        {45, 48, 48, 46, 51, 50, 48, 47, 49, 49, 51, 51, 45, 47, 48, 49, 1000, 1000, 1000, 1000, 47, 45, 46, 46, 49, 44, 47, 51, 47, 45, 45, 45, 1000, 1000, 1000, 1000, 50, 51, 49, 51, 48, 50, 47, 48, 51, 47, 47, 45, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 44, 46, 48, 1000, 1000, 1000, 1000, 48, 44, 46, 1000, 49, 50, 50, 48, 1000, 44, 48, 48, 1000, 1000, 1000, 1000, 47, 46, 45, 51, 47, 44, 47, 44, 1000, 49, 48, 47, 1000, 1000, 1000, 1000, 46, 50, 49, 51, 48, 46, 45, 47, 1000, 51, 44, 47, 1000, 1000, 1000, 1000, 44, 46, 47, 1000, 50, 51, 47, 48, 1000, 46, 46, 44, 50, 44, 45, 44, 50, 49, 1000, 1000, 1000, 47, 49, 1000, 1000, 46, 50, 48, 50, 50, 46, 46, 47, 44, 47, 1000, 48, 48, 44, 51, 1000, 51, 48, 49, 1000, 1000, 1000, 1000, 50, 50, 45, 51, 48, 49, 46, 50, 1000, 47, 50, 48, 1000, 1000, 1000, 1000, 50, 48, 48, 44, 47, 45, 50, 44, 1000, 49, 48, 51, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 47, 51, 45, 47, 48, 45, 45, 44, 47, 50, 48, 50, 51, 50, 44, 44, 51, 48, 44, 49, 45, 49, 50, 44, 51, 51, 48, 47, 49, 47, 48, 50, 48, 46, 50, 45, 48, 46, 50, 45, 51, 49, 47, 46, 46, 46, 46, 44, 50, 51, 45},
};

std::vector<std::array<std::uint32_t, 16*16>> custom_floor {
                    {35, 35, 35, 33, 37, 35, 33, 33, 37, 38, 35, 38, 33, 36, 37, 35, 1000, 1000, 34, 35, 33, 38, 35, 33, 38, 38, 37, 37, 36, 34, 36, 35, 1000, 1000, 35, 37, 35, 34, 33, 33, 36, 37, 34, 33, 36, 38, 35, 38, 1000, 1000, 1000, 1000, 34, 36, 33, 34, 38, 33, 34, 36, 36, 33, 37, 33, 1000, 1000, 1000, 1000, 55, 55, 55, 55, 55, 55, 55, 57, 33, 35, 38, 37, 1000, 1000, 1000, 1000, 56, 55, 55, 55, 55, 55, 58, 57, 34, 34, 36, 36, 1000, 1000, 1000, 1000, 58, 55, 57, 55, 58, 56, 57, 57, 38, 34, 34, 37, 1000, 1000, 1000, 1000, 57, 58, 56, 55, 58, 58, 58, 57, 35, 38, 36, 36, 1000, 1000, 1000, 1000, 55, 56, 57, 58, 56, 55, 56, 57, 38, 37, 36, 37, 1000, 1000, 1000, 1000, 57, 57, 57, 56, 55, 58, 55, 57, 38, 36, 35, 33, 1000, 1000, 1000, 1000, 57, 56, 57, 57, 57, 58, 58, 57, 36, 35, 38, 37, 1000, 1000, 1000, 1000, 56, 57, 57, 56, 55, 57, 56, 57, 38, 35, 33, 34, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 35, 35, 35, 35, 33, 36, 37, 38, 37, 34, 33, 36, 34, 36, 33, 33, 38, 35, 35, 33, 33, 38, 37, 34, 34, 36, 38, 34, 36, 38, 34, 35, 38, 34, 37, 35, 34, 37, 35, 33, 36, 35, 38, 36, 34, 37, 34, 36, 37, 36, 33, 33},
};

std::vector<std::array<std::uint32_t, 16*16>> collisions = {
    {20, 20, 20, 20, 3221225540, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 19, 19, 19, 19, 2147483715, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 19, 27, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 3221225540, 1000, 1000, 19, 19, 16, 19, 5, 5, 5, 19, 5, 5, 5, 5, 19, 2147483715, 1000, 1000, 19, 19, 14, 14, 2147483715, 66, 67, 5, 2147483715, 66, 66, 67, 28, 2147483715, 1000, 1000, 25, 19, 19, 15, 2147483715, 1000, 68, 66, 2147483716, 1000, 1000, 67, 19, 2147483715, 1000, 1000, 15, 19, 19, 26, 2147483715, 1000, 1073741892, 20, 3221225540, 1000, 1000, 67, 27, 2147483715, 1000, 1000, 5, 5, 5, 5, 2147483715, 1073741892, 20, 19, 20, 3221225540, 1073741892, 20, 19, 2147483715, 1000, 1000, 66, 66, 66, 66, 2147483716, 67, 5, 19, 5, 2147483715, 67, 5, 16, 2147483715, 1000, 1000, 20, 20, 20, 20, 3221225540, 68, 67, 5, 2147483715, 2147483716, 68, 67, 19, 2147483715, 1000, 1000, 19, 19, 19, 19, 2147483715, 1000, 68, 66, 2147483716, 1000, 1000, 67, 19, 2147483715, 1000, 1000, 25, 28, 15, 25, 20, 20, 20, 20, 20, 20, 20, 20, 27, 2147483715, 1000, 1000, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2147483715, 1000, 1000, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 2147483716, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
					 {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 70, 1000, 1000, 1000, 70, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 66, 1000, 66, 7, 66, 1000, 1000, 66, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 2147483720, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 2147483721, 1000, 71, 1000, 1000, 1000, 1000, 1000, 2147483655, 71, 1000, 2147483720, 1000, 72, 1000, 1000, 2147483721, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483721, 7, 1000, 2147483655, 73, 2147483721, 7, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 66, 7, 66, 1000, 1000, 66, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 2147483720, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 2147483721, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483655, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
					 {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 2147483720, 2147483655, 71, 1000, 1000, 2147483720, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 73, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 70, 71, 1000, 1000, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483721, 2147483655, 72, 1000, 1000, 2147483720, 1000, 72, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 73, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
					 {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 72, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 71, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},

};
std::vector<std::array<std::uint32_t, 16*16>> wall_tops = {
{1, 1, 1, 1, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483660, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483672, 23, 23, 23, 1, 23, 23, 23, 23, 1, 1000, 1000, 1000, 1000, 1000, 1000, 2147483659, 1000, 1000, 1000, 23, 1000, 1000, 1000, 1000, 11, 1000, 1000, 1000, 1000, 1000, 1000, 2147483661, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 11, 1000, 1000, 1000, 1000, 1000, 1000, 2147483661, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 12, 1000, 1000, 1000, 23, 23, 23, 23, 1000, 1000, 1000, 1, 1000, 1000, 1000, 1000, 11, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 23, 24, 23, 1000, 1000, 23, 2147483660, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 23, 1000, 1000, 1000, 1000, 11, 1000, 1000, 1000, 1, 1, 1, 1, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 11, 1000, 1000, 1000, 1000, 1000, 1000, 2147483661, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 12, 1000, 1000, 1000, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
					 {1000, 1000, 1000, 2147483648, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483650, 1, 1, 1, 24, 1, 1, 1, 1, 2147483648, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 13, 1000, 1000, 1000, 1000, 2147483661, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483659, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483661, 1000, 1000, 1000, 1000, 1000, 1000, 2147483659, 1000, 1000, 1000, 0, 1000, 1000, 1000, 1000, 2147483660, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1, 2147483672, 1, 1000, 1000, 1, 24, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 12, 1000, 1000, 1000, 1000, 2147483660, 1000, 1000, 1000, 1000, 1000, 1000, 2147483648, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483661, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483660, 1000, 1000, 1000, 1000, 1000, 1000, 2147483650, 1, 1, 1, 1, 1, 1, 1, 1, 2147483659, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
					 {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483672, 1000, 1000, 1000, 1000, 24, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483659, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483648, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0, 2, 2147483648, 1000, 1000, 0, 2, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483659, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
					 {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 2147483650, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
};
std::generator<std::uint32_t> generate_tileids(int bound) {
    for (auto const& i : std::views::iota(1, bound+1)) {
        co_yield i;
        co_yield i;
        co_yield i;
        co_yield i;
    }
}

void MyApp::setup() {
    WindowedApp::setup();
    // elog::Elogger::global_logger().install_log_handler(elog::handlers::sd_journal_handler);
    elog::Elogger::global_logger().install_filter_rules("engine.backends.*.bindings{debug}=false");
    // if (!m_virtualFileSystem->CreateFileSystem<dwarfs_fs>("/data", "data/assets.dwarfs")) {
    //     std::println(elogDebug(), "Could not create dwarfs filesystem");
    // }
    if (!m_virtualFileSystem->CreateFileSystem<vfspp::NativeFileSystem>("/data", "data")) {
        std::println(elogDebug(), "Could not create native filesystem");
    }


    camera = OrthographicCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.set_far_plane(1.0f);
    camera.set_near_plane(-1.0f);
    camera.set_position(glm::vec3(0.0f, 0.0f, 1.0f));

    // program = m_graphics_backend->load_program_from_spirv(ShaderHandle::create<sprite_render_shader>());


    auto vertices_buffer = m_graphics_backend->create_buffer(2);
    vertices_buffer[0]->upload(static_mesh.first.size() * sizeof(Vertex2) + 1, static_mesh.first.data(),
                               GL_DYNAMIC_DRAW);
    vertices_buffer[0]->set_name(std::string_view("static_mesh_vertices"));
    vertices_buffer[1]->upload(static_mesh.second.size() * sizeof(std::uint32_t) + 1, static_mesh.second.data(),
                               GL_DYNAMIC_DRAW);
    vertices_buffer[1]->set_name(std::string_view("static_mesh_indicies"));
    tileid_buffer1 = m_graphics_backend->create_buffer();
    tileid_buffer2 = m_graphics_backend->create_buffer();
    std::vector<std::uint32_t> tile_ids;
    tile_ids.reserve(16*16*4);
    tile_ids.append_range(generate_tileids(16 * 16));
    // std::ranges::iota(tile_ids, 1);
    std::array<std::uint32_t, 16 * 16 * 4> tile_ids2{};
    tile_ids2.fill(22);

    tileid_buffer1->set_name(std::string_view("tileid_buffer1"));
    tileid_buffer2->set_name(std::string_view("tileid_buffer2"));
    // tileid_buffer->allocate(sizeof(std::uint32_t)*16*16*4+sizeof(std::uint32_t)*16*16*4, GL_DYNAMIC_DRAW);
    tileid_buffer1->upload(sizeof(std::uint32_t) * 16 * 16 * 4, tile_ids.data(), GL_DYNAMIC_DRAW);
    tileid_buffer2->upload(sizeof(std::uint32_t) * 16 * 16 * 4, tile_ids2.data(), GL_DYNAMIC_DRAW);

    vao2 = m_graphics_backend->create_vao("layer1_vao");
    vao2->set_ebo(vertices_buffer[1]);

    const auto vbuffer = vao2->add_vertex_data_buffer(vertices_buffer[0], sizeof(Vertex2));

    vao2->attrib_format(sprite_render_shader2::get_input_location("vertex"), GL_FLOAT, false);
    vao2->attrib_format(sprite_render_shader2::get_input_location("texCoord"), GL_FLOAT, false, offsetof(Vertex2, uv));

    if (auto hnd = vbuffer.lock()) {
        vao2->bind_buffer_to_attrib(hnd, sprite_render_shader2::get_input_location("vertex").location);
        vao2->bind_buffer_to_attrib(hnd, sprite_render_shader2::get_input_location("texCoord").location);
    }
    tileid_buffer1_handle = vao2->add_vertex_data_buffer(tileid_buffer1, sizeof(std::uint32_t));
    tileid_buffer2_handle = vao2->add_vertex_data_buffer(tileid_buffer2, sizeof(std::uint32_t));

    vao2->attrib_format(sprite_render_shader2::get_input_location("tileId"), GL_UNSIGNED_INT, false);
    // vao2->attrib_divisor(tilebuffer, 1);
    if (auto tb = tileid_buffer1_handle.lock()) {
        vao2->bind_buffer_to_attrib(tb, sprite_render_shader2::get_input_location("tileId").location);
    }


    // vfspp::IFilePtr file = m_virtualFileSystem->OpenFile("/data/sprite_render_shader2.spv", vfspp::IFile::FileMode::Read);
    //
    // size_t size = file->Size();
    // std::vector<uint8_t> shader_binary_data;
    // file->Read(shader_binary_data, size);
    // file->Close();
    program2 = m_graphics_backend->load_program_from_spirv(ShaderHandle::create<sprite_render_shader2>());

    program2->set_uniform(sprite_render_shader2::get_uniform_location("model"),
                          glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)), glm::vec3(32.0f, 32.0f, 1.0f)));
    program2->set_uniform(sprite_render_shader2::get_uniform_location("view"), camera.view());
    program2->set_uniform(sprite_render_shader2::get_uniform_location("projection"), camera.projection());

    vfspp::IFilePtr texture_file = m_virtualFileSystem->OpenFile("/data/TopDown_by_deepnight.ktx2", vfspp::IFile::FileMode::Read);
    texture2 = m_graphics_backend->create_texture_from_file(texture_file);
    texture_file->Close();
    texture2->set_parameter(TextureParameters::MAG_FILTER, GL_NEAREST);
    texture2->set_parameter(TextureParameters::MIN_FILTER, GL_NEAREST);

    auto f = m_virtualFileSystem->OpenFile("/data/tilemap", vfspp::IFile::FileMode::Read);
    std::vector<uint8_t> text(f->Size());
    f->Read(text);
    f->Close();
    std::string tilemapstring(text.begin(), text.end());

    tilemap_parse_temp temp_parser;
    std::vector<uint32_t> tilemap_tileids = temp_parser.parse(tilemapstring, {{'#', 25}, {'_', 22}});
    std::println(elogInfo(), "size: {}", tilemap_tileids.size());
    // std::println(elogInfo(), "tilemap_tileids: {}", tilemap_tileids);
    layer1 = new TilemapLayer(m_graphics_backend.get(), 16, 16, 1);

    std::vector<std::array<std::uint32_t, 16*16*4>> data;

    {
        std::array<std::uint32_t, 16*16*4> tile{};

        for (auto const& row : tiledata) {
            size_t i = 0;


            for (auto const& col : row) {
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
            }
            data.push_back(tile);

        }
    }


    // std::println(elogInfo(), "data0 size: {}", data[0].size());
    // std::println(elogInfo(), "data0: {}", data[0]);

    layer1->set_tileid_for_chunk({0,0},data[0], {});
    // std::println(elogInfo(), "data1 size: {}", data[1].size());

    // std::println(elogInfo(), "data1: {}", data[1]);
    // layer1->set_tileid_for_chunk({1,0},data[1]);
    layer1->finalise(m_graphics_backend.get());
    std::vector<std::array<std::uint32_t, 16*16*4>> custom_floor_data;
    {
        std::array<std::uint32_t, 16*16*4> tile{};

        for (auto const& row : custom_floor) {
            size_t i = 0;

            for (auto const& col : row) {
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
            }
            custom_floor_data.push_back(tile);

        }
    }
    layer2 = new TilemapLayer(m_graphics_backend.get(), 16, 16, 2);
    // std::println(elogInfo(), "floor_data size: {}", custom_floor_data[0].size());
    // std::println(elogInfo(), "floor_data: {}", custom_floor_data[0]);
    layer2->set_tileid_for_chunk({0,0}, custom_floor_data[0], {});
    layer2->finalise(m_graphics_backend.get());
    std::vector<std::array<std::uint32_t, 16*16*4>> collisions_data;
    {
        std::array<std::uint32_t, 16*16*4> tile{};

        for (auto const& row : collisions) {
            size_t i = 0;

            for (auto const& col : row) {
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
            }
            collisions_data.push_back(tile);


        }
        // std::swap(collisions_data.front(), collisions_data.back());
        // std::array<std::uint32_t, 16*16*4> tile2 = collisions_data.back();
        // collisions_data.pop_back();
        // collisions_data.insert(collisions_data.begin(), tile2);
    }
    layer3 = new TilemapLayer(m_graphics_backend.get(), 16, 16, 3);
    // std::println(elogInfo(), "floor_data size: {}", custom_floor_data[0].size());
    // std::println(elogInfo(), "floor_data: {}", custom_floor_data[0]);
    layer3->set_tileid_for_chunk({0,0}, collisions_data[0], std::span(collisions_data).subspan(1));
    layer3->finalise(m_graphics_backend.get());

    std::vector<std::array<std::uint32_t, 16*16*4>> wall_tops_data;
    {
        std::array<std::uint32_t, 16*16*4> tile{};

        for (auto const& row : wall_tops) {
            size_t i = 0;

            for (auto const& col : row) {
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
                tile[i] = col;
                i++;
            }
            wall_tops_data.push_back(tile);


        }
        // std::swap(collisions_data.front(), collisions_data.back());
        // std::array<std::uint32_t, 16*16*4> tile2 = collisions_data.back();
        // collisions_data.pop_back();
        // collisions_data.insert(collisions_data.begin(), tile2);
    }
    layer4 = new TilemapLayer(m_graphics_backend.get(), 16, 16, 4);
    // std::println(elogInfo(), "floor_data size: {}", custom_floor_data[0].size());
    // std::println(elogInfo(), "floor_data: {}", custom_floor_data[0]);
    layer4->set_tileid_for_chunk({0,0}, wall_tops_data[0], std::span(wall_tops_data).subspan(1));
    layer4->finalise(m_graphics_backend.get());



    m_renderer = TilemapRenderer::create(m_graphics_backend.get());

    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // ImGui::StyleColorsDark();
    // m_imgui_renderer = std::make_shared<ImGui_Renderer>();
    // Get a reference to graphics_backend. Not the same as unique_ptr<>.get()
    // m_imgui_renderer->initialise(*m_graphics_backend);

    // glEnable(GL_FRAMEBUFFER_SRGB);
    #ifdef DEBUG
    // Make it easier to find all the loading and setup gl calls in RenderDoc.
    glFinish();
    #endif
    glEnable(GL_DEPTH_TEST);
}

void MyApp::shutdown() {
    WindowedApp::shutdown();
    delete layer1;
    delete layer2;
    delete layer3;
    delete layer4;
    layer1 = nullptr;
    layer2 = nullptr;
    layer3 = nullptr;
    layer4 = nullptr;
}


void MyApp::update(float dt) {

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (depth_test_dirty) {
        if (depth_test) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        depth_test_dirty = false;
    }
    if (srgb_fb_dirty) {
        if (srgb_fb) {
            glEnable(GL_FRAMEBUFFER_SRGB);
        } else {
            glDisable(GL_FRAMEBUFFER_SRGB);
        }
        srgb_fb_dirty = false;
    }
    // ImGui::NewFrame();
    // m_graphics_backend->begin_frame(dt);
    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow();
    {
        ImGui::Begin("Backend State");
        if (ImGui::Checkbox("Enable depth test", &depth_test)) {
            depth_test_dirty = true;
        }
        if (ImGui::Checkbox("Enable SRGB framebuffer", &srgb_fb)) {
            srgb_fb_dirty = true;
        }
        ImGui::End();
        // app_log.Draw("Example: Log");
        // BindingHandle handle(texture2, 0);
        //
        m_renderer->begin_drawing(4, camera);
        m_renderer->draw(m_graphics_backend.get(), *layer4);
        m_renderer->draw(m_graphics_backend.get(), *layer3);
        m_renderer->draw(m_graphics_backend.get(), *layer2);
        m_renderer->draw(m_graphics_backend.get(), *layer1);
        m_renderer->end_drawing();

        // BindingHandle vao_binding(vao2);
        // BindingHandle program_binding(program2);
        //
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("model"),
        //                       glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)),
        //                                  glm::vec3(32.0f, 32.0f, 1.0f)));
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("layer"), 1.0f);
        //
        // if (auto tb = tileid_buffer1_handle.lock()) {
        //     vao2->bind_buffer_to_attrib(tb, sprite_render_shader2::get_input_location("tileId").location);
        //     // glDrawElements(GL_TRIANGLES, 16 * 16 * 6, GL_UNSIGNED_INT, nullptr);
        //     m_graphics_backend->draw_elements(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0);
        //
        // }
        // // glVertexArrayAttribBinding(vao2->get_id(), sprite_render_shader2::get_input_location("tileId").location, tileid_binding_index1);
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("model"),
        //                       glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(32.0f * 16, 0.0f, 0.0f)),
        //                                  glm::vec3(32.0f, 32.0f, 1.0f)));
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("layer"), 0.5f);
        // if (auto tb = tileid_buffer2_handle.lock()) {
        //     vao2->bind_buffer_to_attrib(tb, sprite_render_shader2::get_input_location("tileId").location);
        //     // m_graphics_backend->draw_elements(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0);
        //     glDrawElements(GL_TRIANGLES, 16 * 16 * 6, GL_UNSIGNED_INT, nullptr);
        // }
    }
    ImGui::ShowDemoWindow();
}
