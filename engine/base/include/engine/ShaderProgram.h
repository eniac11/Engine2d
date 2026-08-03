#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <engine/ShaderResources.h>
#include "graphics/Buffer.h"
// #include "RAII/program_binding.h"
#include <memory>
#include <vector>
#include <ranges>


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

        virtual GLuint get_id() const = 0;




        std::vector<UBOHandle>& get_ubos();

        virtual void add_ubo(int shader_ubo_block_binding, std::shared_ptr<Buffer> buffer) = 0;
        virtual std::unique_ptr<memo_type> bind() const = 0;

        #include <engine/uniforms.h>

    protected:



    protected:

        std::vector<UBOHandle> m_ubos;
};


