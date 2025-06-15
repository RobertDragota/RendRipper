#pragma once
#include <vector>
#include <string>
#include <memory>
#include <assimp/scene.h>
#include "Mesh.h"

/**
 * @brief Loads textures referenced by Assimp materials.
 */
class TextureLoader {
public:
    /**
     * @brief Load all textures of a specific type from a material.
     */
    std::vector<std::shared_ptr<Texture>> LoadMaterialTextures(aiMaterial* mat,
                                               aiTextureType type,
                                               const std::string& typeName,
                                               const std::string& directory) const;
};
