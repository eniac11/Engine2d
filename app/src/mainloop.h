#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <engine/Camera.h>
#include <engine/ShaderProgram.h>
#include <engine/graphics/Texture.h>
#include <engine/app.h>
#include <engine/graphics/Buffer.h>
#include <engine/graphics/VertexArray.h>
#include <memory>
#include <engine/2d/Chunk.h>

constexpr int WINDOW_HEIGHT = 600;
constexpr int WINDOW_WIDTH = 800;

class MyApp : public WindowedApp {
    public:
        MyApp() : WindowedApp(WINDOW_WIDTH, WINDOW_HEIGHT, "MyApp") {
        }

        // std::string const name() override final { return std::string("MyApp"); }
        void update(float dt) override;
        void setup() override;
        void shutdown() override;

    private:
        OrthographicCamera camera;
        std::shared_ptr<ShaderProgram> program2;
        std::shared_ptr<Texture> texture2;
        std::shared_ptr<Buffer> tileid_buffer1;
        std::shared_ptr<Buffer> tileid_buffer2;
        std::weak_ptr<VertexArray::BufferHandle> tileid_buffer1_handle;
        std::weak_ptr<VertexArray::BufferHandle> tileid_buffer2_handle;
        std::shared_ptr<VertexArray> vao;
        std::shared_ptr<VertexArray> vao2;
        TilemapLayer* layer1= nullptr;
        std::shared_ptr<TilemapRenderer> m_renderer;
};

void set_window_hints();

bool check_extensions();

void run(GLFWwindow* window);
