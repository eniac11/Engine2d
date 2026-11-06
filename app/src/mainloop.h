#pragma once
#include <engine/Camera.h>
#include <engine/ShaderProgram.h>
#include <engine/graphics/Texture.h>
#include <engine/app.h>
#include <engine/graphics/Buffer.h>
#include <engine/graphics/VertexArray.h>
#include <engine/2d/Chunk.h>

#include <memory>
#include <engine/2d/sprite_renderer.h>
#include <ldtk_parser/project.h>
#include <entt/entity/registry.hpp>

struct Sprite;
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
        TilemapLayer* layer2= nullptr;
        TilemapLayer* layer3= nullptr;
        TilemapLayer* layer4= nullptr;
        std::shared_ptr<TilemapRenderer> m_renderer;
        bool depth_test = true;
        bool depth_test_dirty =false;
        bool srgb_fb_dirty = false;
        bool srgb_fb = true;
        std::unique_ptr<sprite_renderer> m_sprite_render;
        std::vector<Sprite> m_sprites;
        ldtk::Project project_;
        entt::registry registry_;
};

void set_window_hints();

bool check_extensions();

