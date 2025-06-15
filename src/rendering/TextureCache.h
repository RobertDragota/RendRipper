#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Mesh.h"

/**
 * @brief Caches textures to avoid duplicate uploads.
 */
class TextureCache {
public:
    /** Load a texture from disk or return a cached instance. */
    static std::shared_ptr<Texture> Get(const std::string& path, const std::string& type);
    /** Create or retrieve a 1x1 solid color texture. */
    static std::shared_ptr<Texture> GetSolidColorTexture(unsigned char r,
                                                         unsigned char g,
                                                         unsigned char b,
                                                         unsigned char a);
    /** Clear all cached textures. */
    static void Clear();
private:
    static std::unordered_map<std::string, std::weak_ptr<Texture>> cache_;
};
