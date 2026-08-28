#pragma once
#include <GL/glew.h>
#include <cassert>

#include <filesystem>
#include <ktx.h>
#include <vfspp/IFile.h>

// #include "../RAII/texture_binding.h"

class texture_exception : public std::runtime_error {
    public:
        explicit texture_exception(const std::string &basic_string)
            : runtime_error(basic_string) {
        }
};

enum class TextureParameters {
    MAG_FILTER,
    MIN_FILTER
};

constexpr const char* to_string(TextureParameters e) {
    switch (e) {
        case TextureParameters::MAG_FILTER: return "MAG_FILTER";
        case TextureParameters::MIN_FILTER: return "MIN_FILTER";
        default: return "unknown";
    }
}

class Texture {
    // TODO(hadley): separate the actual texture created by the backend and the actual data source eg. memory, ktx
    public:
        struct Memo {
            virtual ~Memo() =default;
        };
        using memo_type = Memo;
        // Texture(int id, int width, int height);
        // Texture(int id, ktxTexture* texture);
        virtual ~Texture() = default;
        [[nodiscard]] virtual std::uint32_t get_id() const = 0;


        [[nodiscard]] virtual std::uint32_t width() const = 0;
        [[nodiscard]] virtual std::uint32_t height() const = 0;
        [[nodiscard]] bool is_valid() const {return m_valid;}

        [[nodiscard]] virtual bool has_layers() = 0;
        [[nodiscard]] virtual std::uint32_t layers() = 0;

        virtual std::unique_ptr<memo_type> bind(int binding_index) const = 0;

        virtual void parameter(TextureParameters param, int value) = 0;

    protected:
        void set_valid(bool const valid) {m_valid=valid;};
    private:
        bool m_valid = false;

        // virtual void load_data(const void* data) const = 0;

};

ktxTexture* read_texture_from_file(std::filesystem::path const &path);
ktxTexture* read_texture_from_file(vfspp::IFilePtr file);