#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Mesh.h"

class Model {
public:
    explicit Model(const std::string& path);
    ~Model();

    // non‑copyable
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    // movable
    Model(Model&&) noexcept;
    Model& operator=(Model&&) noexcept;

    void Draw(const Shader& shader) const;

    // bounding info
    glm::vec3 center;
    float radius;

private:
    std::string directory;
    std::vector<Mesh> meshes;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                              const std::string& typeName);
    void computeBounds();
};
