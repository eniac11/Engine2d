#pragma once
#include <memory>
#include <string>

#include "graphics/backend.h"
#include <vfspp/VirtualFileSystem.hpp>

class App {

    public:
        App();
        virtual ~App() = default;
        std::string name() const;
        void set_name(std::string name);

        virtual void run();
        static App* instance();
    protected:
        vfspp::VirtualFileSystemPtr m_virtualFileSystem;


    private:
        static App* s_instance;
        std::string m_name{};
};


class Window {
    public:
        virtual ~Window() {};
        virtual bool window_should_close() const = 0;
        virtual void close_window() = 0;
        virtual bool is_open() = 0;
        virtual void create(int width, int height, std::string const& title) = 0;
        virtual void update() = 0;
        virtual float deltatime() = 0;
        virtual ENGINE_PFNGLGETPROCADDRESS* getProcAdress() = 0;

    protected:
        int m_width;
        int m_height;
        std::string m_title;
        float m_deltaTime = 0.0f; // Time between current frame and last frame
        float m_lastFrame = 0.0f; // Time of last frame
};

class WindowedApp : public App {
    public:
        WindowedApp(int width, int height, std::string const& title) :
            m_title(title), m_height(height), m_width(width) {

            };
        void setup_window(std::unique_ptr<Window> window);
        void setup_graphics_backend(std::unique_ptr<graphics::GraphicsBackend>&& backend);
        virtual void setup() = 0;
        virtual void update(float dt) = 0;
        virtual void shutdown() {}
        void run() override final;

    protected:
        std::unique_ptr<Window> m_window;
        std::unique_ptr<graphics::GraphicsBackend> m_graphics_backend;
        int m_height;
        int m_width;
        std::string m_title;
};
