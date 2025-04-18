#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <glm/glm.hpp>

Model::Model(const std::string& path) { loadModel(path); }

void Model::Draw(Shader& shader) { for (auto& mesh : meshes) mesh.Draw(shader); }

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        throw std::runtime_error(importer.GetErrorString());
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;
    std::vector<Texture> texs;
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v;
        v.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        v.Normal   = mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                                        : glm::vec3(0.0f);
        v.TexCoord = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                                             : glm::vec2(0.0f);
        verts.push_back(v);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            inds.push_back(mesh->mFaces[i].mIndices[j]);
    // No textures loaded -> meshes will use white texture bound in main
    return Mesh(verts, inds, texs);
}