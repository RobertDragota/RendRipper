#include "TextureLoader.h"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::vector<Texture> TextureLoader::LoadMaterialTextures(aiMaterial* mat,
                                                         aiTextureType type,
                                                         const std::string& typeName,
                                                         const std::string& directory) const
{
    std::vector<Texture> ret;
    for (unsigned i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string filename = directory + "/" + str.C_Str();
        Texture tex;
        tex.type = typeName;
        tex.path = filename;
        glGenTextures(1, &tex.id);
        int w, h, n;
        unsigned char* data = stbi_load(filename.c_str(), &w, &h, &n, 0);
        if (data) {
            GLenum fmt = (n == 3 ? GL_RGB : GL_RGBA);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
            ret.push_back(tex);
        }
    }
    return ret;
}
