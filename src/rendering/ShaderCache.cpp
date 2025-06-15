#include "ShaderCache.h"
#include <sstream>

/**
 * @file ShaderCache.cpp
 * @brief Implementation of the simple shader cache.
 */

std::unordered_map<std::string, std::weak_ptr<Shader>> ShaderCache::cache_;

/// Combine shader paths into a unique cache key.
static std::string makeKey(const std::string& v, const std::string& f, const std::string& g){
    std::ostringstream oss;
    oss << v << '|' << f << '|' << g;
    return oss.str();
}

/** Retrieve or load a shader program. */
std::shared_ptr<Shader> ShaderCache::Get(const std::string& vert,
                                         const std::string& frag,
                                         const std::string& geom)
{
    std::string key = makeKey(vert, frag, geom);
    auto it = cache_.find(key);
    if(it != cache_.end()){
        if(auto sp = it->second.lock()) return sp;
    }
    const char* v = vert.empty() ? nullptr : vert.c_str();
    const char* f = frag.empty() ? nullptr : frag.c_str();
    const char* g = geom.empty() ? nullptr : geom.c_str();
    auto shader = std::make_shared<Shader>(v,f,g);
    cache_[key] = shader;
    return shader;
}

/** Clear the shader cache. */
void ShaderCache::Clear(){
    cache_.clear();
}
