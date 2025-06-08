#include "TextureCache.h"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::unordered_map<std::string, std::weak_ptr<Texture>> TextureCache::cache_;

std::shared_ptr<Texture> TextureCache::Get(const std::string& path, const std::string& type){
    auto it = cache_.find(path);
    if(it != cache_.end()){
        if(auto sp = it->second.lock()) return sp;
    }
    int w,h,n;
    unsigned char* data = stbi_load(path.c_str(), &w,&h,&n, 0);
    if(!data){
        return std::make_shared<Texture>();
    }
    auto tex = std::make_shared<Texture>();
    tex->type = type;
    tex->path = path;
    glGenTextures(1, &tex->id);
    GLenum fmt = (n==3 ? GL_RGB : GL_RGBA);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    cache_[path] = tex;
    return tex;
}

void TextureCache::Clear(){
    cache_.clear();
}
