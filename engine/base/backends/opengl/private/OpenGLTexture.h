
#pragma once
#include <engine/graphics/Texture.h>

class OpenGLTexture final : public Texture {
    public:
        static std::shared_ptr<OpenGLTexture> create();
        // static std::shared_ptr<OpenGLTexture> create_texture_from_file(const std::filesystem::path& filepath);
        // static std::shared_ptr<OpenGLTexture> create_texture_from_memory(const void* data, int width, int height, int channels = 4);

        ~OpenGLTexture() override;
        [[nodiscard]] std::uint32_t get_id() const override;

        [[nodiscard]] std::uint32_t width() const override;
        [[nodiscard]] std::uint32_t height() const override;

        [[nodiscard]] bool has_layers() override;
        [[nodiscard]] std::uint32_t layers() override;

        void load_data(const void *data, const int width, const int height, const int channels);
        void load_data(ktxTexture* texture);

        void get_parameter(TextureParameters param, int& value) const override;
        void get_parameter(TextureParameters param, std::uint32_t& value) const override;
        void set_parameter(TextureParameters param, int value) override;

    private:
        OpenGLTexture(GLuint id);

    public:
        std::unique_ptr<memo_type> bind(int binding_index) const override;

    private:
        std::uint32_t m_id{};
        std::uint32_t m_width{};
        std::uint32_t m_height{};
        int m_channels{};
        ktxTexture* m_texture = nullptr;
};
