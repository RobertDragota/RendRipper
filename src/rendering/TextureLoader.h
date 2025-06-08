#pragma once
#include <vector>
#include <string>
#include <memory>
#include <assimp/scene.h>
#include "Mesh.h"

class TextureLoader {
public:
    std::vector<std::shared_ptr<Texture>> LoadMaterialTextures(aiMaterial* mat,
                                               aiTextureType type,
                                               const std::string& typeName,
                                               const std::string& directory) const;
};
