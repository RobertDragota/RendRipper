#include "ModelLoader.h"
#include "TextureLoader.h"
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>

std::vector<Mesh> ModelLoader::Load(const std::string& path) const
{
    std::vector<Mesh> meshes;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        throw std::runtime_error(importer.GetErrorString());
    std::string directory = path.substr(0, path.find_last_of("/\\"));
    processNode(scene->mRootNode, scene, directory, meshes);
    return meshes;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, const std::string& directory, std::vector<Mesh>& meshes) const
{
    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh, scene, directory));
    }
    for (unsigned i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, directory, meshes);
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory) const
{
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v;
        v.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        v.norm = mesh->HasNormals()
            ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
            : glm::vec3(0.0f);
        v.uv = mesh->mTextureCoords[0]
            ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            : glm::vec2(0.0f);
        vertices.push_back(v);
    }

    for (unsigned i = 0; i < mesh->mNumFaces; ++i)
        for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        TextureLoader loader;
        auto diffuse = loader.LoadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse", directory);
        auto specular = loader.LoadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular", directory);
        textures.insert(textures.end(), diffuse.begin(), diffuse.end());
        textures.insert(textures.end(), specular.begin(), specular.end());
    }

    return { std::move(vertices), std::move(indices), std::move(textures) };
}
