#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"
#include <assimp/scene.h>

class Model {
public:
    Model(const std::string& path);
    void Draw(const Shader& shader) const;
    glm::vec3 center; float radius;
private:
    std::vector<Mesh> meshes;
    std::string dir;
    void load(const std::string& path);
    void procNode(aiNode*,const aiScene*);
    Mesh procMesh(aiMesh*,const aiScene*);
    std::vector<Texture> loadTex(aiMaterial*,aiTextureType,const std::string&);
    void computeBounds();
};