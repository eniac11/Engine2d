#pragma once
#include <engine/ShaderResources.h>
#include <engine/graphics/Buffer.h>

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>


class ShaderProgram {
    public:
        struct UBOHandle {
            std::shared_ptr<Buffer> buffer;
            int block_binding_index;
            int binding_index;
            bool bound = false;

            void bind();

            void unbind();
        };
        struct Memo {
            virtual ~Memo() =default;
        };
        using memo_type = Memo;

        virtual ~ShaderProgram() = default;

        virtual uint32_t get_id() const = 0;




        std::vector<UBOHandle>& get_ubos();

        virtual void add_ubo(int shader_ubo_block_binding, std::shared_ptr<Buffer> buffer) = 0;
        virtual std::unique_ptr<memo_type> bind() const = 0;

        #include <engine/uniforms.h>

    protected:



    protected:

        std::vector<UBOHandle> m_ubos;
};


