#pragma once
#include <vector>
#include <string>
#include <assimp/scene.h>
#include "Mesh.h"

class ModelLoader {
public:
    std::vector<Mesh> Load(const std::string& path) const;

private:
    void processNode(aiNode* node, const aiScene* scene, const std::string& directory, std::vector<Mesh>& meshes) const;
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory) const;
};
