#include "Model.h"
#include "Shader.h"
#include "glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <algorithm>

Model::Model(const std::string& path) {
    directory = path.substr(0, path.find_last_of("/\\"));
    loadModel(path);
    computeBounds();
}

Model::~Model() {
    // meshes vor elibera texturile și buffer‑ele în destructori
}

Model::Model(Model&& o) noexcept
        : directory(std::move(o.directory)), meshes(std::move(o.meshes)),
          center(o.center), radius(o.radius)
{}

Model& Model::operator=(Model&& o) noexcept {
    if (this != &o) {
        directory = std::move(o.directory);
        meshes = std::move(o.meshes);
        center = o.center;
        radius = o.radius;
    }
    return *this;
}

void Model::Draw(const Shader& shader) const {
    for (auto& m : meshes)
        m.Draw(shader);
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
                                             aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        throw std::runtime_error(importer.GetErrorString());
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh, scene));
    }
    for (unsigned i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;

    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v;
        v.pos  = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        v.norm = mesh->HasNormals()
                 ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                 : glm::vec3(0.0f);
        v.uv   = mesh->mTextureCoords[0]
                 ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                 : glm::vec2(0.0f);
        vertices.push_back(v);
    }
    for (unsigned i = 0; i < mesh->mNumFaces; ++i)
        for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuse  = loadMaterialTextures(mat, aiTextureType_DIFFUSE,  "texture_diffuse");
        auto specular = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), diffuse.begin(), diffuse.end());
        textures.insert(textures.end(), specular.begin(), specular.end());
    }

    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                                 const std::string& typeName)
{
    std::vector<Texture> ret;
    for (unsigned i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string filename = directory + "/" + str.C_Str();
        Texture tex;
        tex.type = typeName;
        tex.path = filename;
        glGenTextures(1, &tex.id);
        int w,h,n;
        unsigned char* data = stbi_load(filename.c_str(), &w,&h,&n,0);
        if (data) {
            GLenum fmt = (n==3? GL_RGB: GL_RGBA);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D,0,fmt,w,h,0,fmt,GL_UNSIGNED_BYTE,data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            stbi_image_free(data);
            ret.push_back(tex);
        }
    }
    return ret;
}

void Model::computeBounds() {
    glm::vec3 mn( FLT_MAX), mx(-FLT_MAX);
    for (auto& mesh : meshes)
        for (auto& v : mesh.getVertices()) {
            mn = glm::min(mn, v.pos);
            mx = glm::max(mx, v.pos);
        }
    center = (mn + mx) * 0.5f;
    radius = glm::length(mx - center);
}
