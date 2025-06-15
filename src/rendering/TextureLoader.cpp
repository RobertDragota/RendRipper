#include "TextureLoader.h"
#include "TextureCache.h"
#include <glad/glad.h>
#include <memory>

/**
 * @file TextureLoader.cpp
 * @brief Functions for loading textures via Assimp.
 */

/** Load material textures of a specific type. */
std::vector<std::shared_ptr<Texture>> TextureLoader::LoadMaterialTextures(aiMaterial* mat,
                                                         aiTextureType type,
                                                         const std::string& typeName,
                                                         const std::string& directory) const
{
    std::vector<std::shared_ptr<Texture>> ret;
    for (unsigned i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string filename = directory + "/" + str.C_Str();
        ret.push_back(TextureCache::Get(filename, typeName));
    }
    return ret;
}
