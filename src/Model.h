#pragma once
#include <vector>
#include <string>
#include "Mesh.h"
#include <assimp/scene.h>

class Model {
public:
    Model(const std::string& path);
    void Draw(Shader& shader);
private:
    std::vector<Mesh> meshes;
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};