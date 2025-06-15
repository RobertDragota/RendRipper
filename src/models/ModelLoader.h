#pragma once
#include <vector>
#include <string>
#include <assimp/scene.h>
#include <memory>
#include "Mesh.h"

/**
 * @brief Helper class for loading meshes using Assimp.
 */
class ModelLoader {
public:
    /**
     * @brief Load all meshes from the specified file path.
     * @param path Path to the model file to read.
     * @return Vector of loaded meshes.
     */
    std::vector<Mesh> Load(const std::string& path) const;

private:
    /**
     * @brief Recursively traverse the scene nodes and fill the mesh list.
     */
    void processNode(aiNode* node, const aiScene* scene,
                     const std::string& directory, std::vector<Mesh>& meshes) const;

    /**
     * @brief Convert an Assimp mesh into our Mesh structure.
     */
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory) const;
};
