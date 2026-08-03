// #define GLM_FORCE_MESSAGES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <engine/app.h>
#include <glm/glm.hpp>
#include <iostream>
#include <ktx.h>
#include <memory>
#include <stdexcept>

#include <engine/elog.h>


#include "mainloop.h"


#include "backward.hpp"
#include "engine/backends/graphics/opengl_backend.h"

namespace backward {

    backward::SignalHandling sh;

} // namespace backward

ELOG_DECLARE_LOGGING_CATEGORY(cEngineWindowing, "engine.windowing.glfw")

class GlfwWindow : public Window {

    public:
        ENGINE_PFNGLGETPROCADDRESS* getProcAdress() override {
            return (ENGINE_PFNGLGETPROCADDRESS*)&glfwGetProcAddress;
        }
        float deltatime() override { return m_deltaTime; }

        void update() override {
            glfwPollEvents();
            glfwSwapBuffers(m_window);
            float currentFrame = glfwGetTime();
            m_deltaTime = currentFrame - m_lastFrame;
            m_lastFrame = currentFrame;
        }

        ~GlfwWindow() override {}
        bool window_should_close() const override { return glfwWindowShouldClose(m_window); }

        void close_window() override { glfwDestroyWindow(m_window); }

        bool is_open() override {
            int visible = glfwGetWindowAttrib(m_window, GLFW_VISIBLE);
            if (visible == GLFW_TRUE) {
                return true;
            } else if (visible == GLFW_FALSE) {
                return false;
            }
            return false;
        }

        void create(int width, int height, const std::string& title) override {
            if (!glfwInit()) {
                const char* description;
                glfwGetError(&description);
                std::println(elogCDebug(cEngineWindowing), "Failed to create GLFW window: {}", description);
                throw std::runtime_error("Failed to initialize GLFW");
            }
            m_height = height;
            m_width = width;
            m_title = title;
            set_window_hints();

            m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
            if (!m_window) {
                const char* description;
                glfwGetError(&description);
                std::println(elogCDebug(cEngineWindowing), "Failed to create GLFW window: {}", description);
                glfwTerminate();
                throw std::runtime_error("Failed to create GLFW window");
            }
            glfwMakeContextCurrent(m_window);

            init_glew();

            if (!check_extensions()) {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
                glfwTerminate();
                std::println(std::cerr, "GPU does not support required extensions");
                exit(EXIT_FAILURE);
            }

            glViewport(0, 0, m_width, m_height);
        }

    private:
        void init_glew() {

            if (const GLenum error = glewInit(); error != GLEW_OK) {
                std::cerr << "Error: " << glewGetErrorString(error) << std::endl;
                glfwTerminate();
                throw std::runtime_error("Failed to initialize GLEW");
            }
        }
        GLFWwindow* m_window;
};

// GLFWwindow* setup_window() {
//     if (!glfwInit()) {
//         const char* description;
//         glfwGetError(&description);
//         std::cerr << "Failed to create GLFW window " << description << std::endl;
//         throw std::runtime_error("Failed to initialize GLFW");
//     }
//
//     set_window_hints();
//
//     GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World!", nullptr, nullptr);
//     if (!window) {
//         const char* description;
//         glfwGetError(&description);
//         std::cerr << "Failed to create GLFW window " << description << std::endl;
//         glfwTerminate();
//         throw std::runtime_error("Failed to create GLFW window");
//     }
//     glfwMakeContextCurrent(window);
//
//     if (!check_extensions()) {
//         glfwSetWindowShouldClose(window, GLFW_TRUE);
//         glfwTerminate();
//         std::println(std::cerr, "GPU does not support required extensions");
//         exit(EXIT_FAILURE);
//     }
//
//     glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
//     return window;
// }

// void init_glew() {
//     if (const GLenum error = glewInit(); error != GLEW_OK) {
//         std::cerr << "Error: " << glewGetErrorString(error) << std::endl;
//         glfwTerminate();
//         throw std::runtime_error("Failed to initialize GLEW");
//     }
// }

int main() {
    MyApp app;
    app.set_name("mygame");
    app.setup_window(std::make_unique<GlfwWindow>());
    app.setup_graphics_backend(std::make_unique<backend::graphics::OpenglBackend>());

    app.setup();


    app.run();

    // GLFWwindow* window = setup_window();
    // init_glew();
    // ktxLoadOpenGL(nullptr);
    //
    // run(window);
    //
    // glfwTerminate();
    return 0;
}
