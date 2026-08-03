#pragma once
#include <string>
#include <exception>
#include <span>
#include <utility>
#include <GL/glew.h>

class unknown_location final : public std::exception {
    public:
    explicit unknown_location(std::string  message) : message(std::move(message)) {}
    const std::string message;
};



struct Resource {
    std::string type;
    std::string name;
    int location;
    int ncomponents;
};

struct EntryPoint {
    GLenum stage;
    std::string name;
};

template <class ShaderT>
concept Shader = requires(ShaderT shader)
{
    {shader.program} -> std::same_as<const unsigned char* const&>;
    {shader.size } -> std::same_as<const unsigned int&>;
    {shader.entry_points()} -> std::same_as<std::span<const EntryPoint>>;
};


class ShaderHandle {
    // template <Shader T>
    public:
        constexpr ShaderHandle(const unsigned char* program, unsigned int size, std::span<const EntryPoint> const entry_points) : program_data(program), program_size(size), program_entry_points(entry_points) {

        }
        template <Shader T>
        static constexpr ShaderHandle create() {
            constexpr T shader;
            return ShaderHandle(shader.program, shader.size, shader.entry_points());
        }

        constexpr const unsigned char* program() const {
        return program_data;
    }
    constexpr unsigned int size() const {
        return program_size;
    }
    constexpr std::span<const EntryPoint> entry_points() const {
        return program_entry_points;
    }
    private:
        // This should only be initialised once, with static program lifetime.
        // static constexpr  T shader{};
        const unsigned char* program_data;
        unsigned int program_size;
        std::span<const EntryPoint> program_entry_points;

};

