#include "engine/models/Model.h"

#include <format>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/io.hpp>

// #include "lighting_shader.h"
#include "engine/logging.h"
#include "engine/Texture.h"
#include "engine/resources/texturestore.h"

class ModelLoader {
    struct TextureData {
        uuid id;
        int material_index;
        aiTextureType type;
    };

    struct LoadingContext {
        Model &model;
        const aiScene *scene;
        int current_matrix_index;
        std::vector<glm::mat4> model_matrices;
        std::vector<TextureData> textures;
    };

    std::shared_ptr<TextureStore> texture_store = nullptr;

    void processNode(LoadingContext &ctx, aiNode *node, glm::mat4 parent_transform);

    std::unique_ptr<Mesh> process_mesh(LoadingContext &ctx, const aiMesh *mesh, glm::mat4 parent_transform);

    std::map<int, uuid> load_material(LoadingContext &ctx, const aiMaterial *material,
                                                         aiTextureType type, const aiScene *scene);

    public:
        ModelLoader() {
        }

        explicit ModelLoader(std::shared_ptr<TextureStore> texture_store) : texture_store(std::move(texture_store)) {}



        void load_model(Model &model, std::filesystem::path const &path);
};


void ModelLoader::load_materials([[maybe_unused]] LoadingContext &ctx, const aiTextureType type,
                                                                  const aiScene *scene) {
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial *material = scene->mMaterials[i];
        for (unsigned int j = 0; j < material->GetTextureCount(type); j++) {
            const aiTexture *texture = scene->GetEmbeddedTexture(std::format("*{}", i).c_str());
            if (texture_store) {
                uuid id = texture_store->register_texture(uuid7(), Texture::create_texture_from_memory(texture->pcData, texture->mWidth, texture->mHeight));
                ctx.textures.emplace_back(id, i, type);
            }
        }
    }
}

std::unique_ptr<Mesh> ModelLoader::process_mesh(LoadingContext &ctx, const aiMesh *mesh, glm::mat4 transform) {
    VectorOfVertices vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture> > textures;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        aiVector3D pos = mesh->mVertices[i];
        auto &p = vertices.positions.emplace_back(pos.x, pos.y, pos.z);
        // std::cout << p << std::endl;

        aiVector3D norm = mesh->mNormals[i];
        vertices.normals.emplace_back(norm.x, norm.y, norm.z);

        aiVector3D tex = mesh->mTextureCoords[0][i];
        vertices.texCoords.emplace_back(tex.x, tex.y);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex > 0) {
        aiMaterial *material = ctx.scene->mMaterials[mesh->mMaterialIndex];
        auto diffuse_maps = load_material(ctx, material, aiTextureType_DIFFUSE, ctx.scene);
        auto specular_maps = load_material(ctx, material, aiTextureType_SPECULAR, ctx.scene);
        textures.append_range(specular_maps);
        textures.append_range(diffuse_maps);
        // auto diffuse_maps = load_material(material, aiTextureType_DIFFUSE, scene);
    }
    ctx.model_matrices.push_back(transform);
    auto mesh_ = std::make_unique<Mesh>(ctx.current_matrix_index, vertices, indices, textures,
                                        std::string(mesh->mName.C_Str()), transform);
    ctx.current_matrix_index++;

    return mesh_;
}

static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4 &from) {
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

void ModelLoader::processNode(LoadingContext &ctx,  aiNode *node, glm::mat4 parent_transform) {
    glm::mat4 transform = parent_transform + ConvertMatrixToGLMFormat(node->mTransformation);
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = ctx.scene->mMeshes[node->mMeshes[i]];
        ctx.model.meshes.push_back(process_mesh(ctx, mesh, transform));
    }

    // TODO: Maybe this is a good place to use `OpenMP` to parallelize model decoding however there are two problems that
    //       need to be resolved.
    //       1. Loading ctx.current_matrix_index is not thread safe should be atomically incremented.
    //       2. When constructing meshes the constructor calls setupMeshes() which in turn calls to opengl, opengl is NOT
    //          thread safe and its functions cannot be called from different thread. setupMeshes() should be done after
    //          this step.
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(ctx, node->mChildren[i], transform);
    }
}

void ModelLoader::load_model(Model &model, std::filesystem::path const &path) {
    if (!(std::filesystem::exists(path) && std::filesystem::is_regular_file(path))) {
        throw model_exception(std::format("Could not load model from '{}' did not exist or was not a file",
                                          path.string()));
    }

    Assimp::Importer aiImporter;

    const aiScene *scene = aiImporter.ReadFile(path.string(),
                                               aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                               aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw model_exception(std::format("ASSIMP import error: {}", aiImporter.GetErrorString()));
    }
    LoadingContext ctx{model, scene, 0};

    processNode(ctx, scene->mRootNode, glm::mat4(1.0f));
    model.model_matrix_buffer->allocate(ctx.model_matrices.size()*sizeof(glm::mat4)+sizeof(int)+(16-sizeof(int)), GL_STATIC_DRAW);
    int const size = static_cast<int>(ctx.model_matrices.size());
    model.model_matrix_buffer->upload_subdata(sizeof(int), 0, &size);
    model.model_matrix_buffer->upload_subdata(ctx.model_matrices.size() * sizeof(glm::mat4), 16, ctx.model_matrices.data());
}


Model Model::load_model(std::filesystem::path const & path) {
    Model model;
    model.model_matrix_buffer = Buffer::create();
    ModelLoader loader;
    loader.load_model(model, path);

    return model;
}

void Model::draw() const {
    for (const auto &mesh: meshes) {
        mesh->draw();
    }
}
