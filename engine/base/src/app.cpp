#include <atomic>
#include <engine/app.h>
#include <ktx.h>
#include <engine/elog.h>
#include <engine/StandardDirectories.h>
#include <vfspp/NativeFileSystem.hpp>

App* App::s_instance = nullptr;

App::App() {
    std::atomic_ref<App*>(App::s_instance).store(this, std::memory_order::relaxed);
    m_virtualFileSystem = std::make_shared<vfspp::VirtualFileSystem>();

}

std::string App::name() const {
    return m_name;
}

void App::set_name(std::string name) {
    m_name = std::move(name);
}

void App::run() {
    StandardDirectories stdDirectories;
    for (auto const& path : stdDirectories.get_user_dirs(StandardDirectories::KnownDirectories::Data, name()) | std::views::reverse) {
        if (std::filesystem::exists(path)) {
            if (!m_virtualFileSystem->CreateFileSystem<vfspp::NativeFileSystem>("/app/data", path.string())) {
                std::println(elogDebug(), "Could not create vfs");
            }

        }
    }
}

App* App::instance() { return std::atomic_ref<App*>(App::s_instance).load(std::memory_order::relaxed); }

void WindowedApp::setup_window(std::unique_ptr<Window> window) {
    m_window = std::move(window);
    m_window->create(m_width, m_height, m_title);
}

void WindowedApp::setup_graphics_backend(std::unique_ptr<graphics::GraphicsBackend>&& backend) {
    m_graphics_backend = std::move(backend);
    m_graphics_backend->initialise(m_window->getProcAdress());
}

void WindowedApp::run() {
    this->App::run();
    while (!m_window->window_should_close()) {
        update(m_window->deltatime());
        m_window->update();
    }
    shutdown();
}
