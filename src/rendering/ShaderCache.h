#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Shader.h"

/**
 * @brief Simple cache to avoid recompiling identical shader programs.
 */
class ShaderCache {
public:
    /** Obtain a shared Shader instance, creating it if needed. */
    static std::shared_ptr<Shader> Get(const std::string& vert,
                                       const std::string& frag,
                                       const std::string& geom = "");
    /** Clear all cached shaders. */
    static void Clear();
private:
    static std::unordered_map<std::string, std::weak_ptr<Shader>> cache_;
};
