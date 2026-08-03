#pragma once
#include <filesystem>
#include <vector>

#include "Mesh.h"
#include "engine/graphics/ubo.h"

class model_exception : public std::runtime_error {
    public:
        explicit model_exception(const std::string &basic_string)
            : runtime_error(basic_string) {
        }
};

class ModelLoader;

struct ModelMatrixData {
    int length;
    alignas(16) glm::mat4 matrices;
};

static_assert(offsetof(ModelMatrixData, matrices) == 16, "Matrices must be alligned to 16");

class Model {
    friend class ModelLoader;
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::shared_ptr<Buffer> model_matrix_buffer;

    public:
        std::vector<std::unique_ptr<Mesh>> const& getMeshes() {
            return meshes;
        }
        // FIXME: This is a temporary solution to make easy to write a proof of concept before SSBO's and UBO's get
        //        there own constructs.
        std::shared_ptr<Buffer> getModelMatrixBuffer() {
            return model_matrix_buffer;
        }
        static Model load_model(std::filesystem::path const& path);
        void draw() const;

};
