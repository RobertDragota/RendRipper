#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Shader.h"

class ShaderCache {
public:
    static std::shared_ptr<Shader> Get(const std::string& vert,
                                       const std::string& frag,
                                       const std::string& geom = "");
    static void Clear();
private:
    static std::unordered_map<std::string, std::weak_ptr<Shader>> cache_;
};
