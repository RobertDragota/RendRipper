#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Mesh.h"

class TextureCache {
public:
    static std::shared_ptr<Texture> Get(const std::string& path, const std::string& type);
    static void Clear();
private:
    static std::unordered_map<std::string, std::weak_ptr<Texture>> cache_;
};
