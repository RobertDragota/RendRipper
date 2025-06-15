#include "Mesh.h"
#include <glad/glad.h>

/**
 * @file Texture.cpp
 * @brief Implementation for the simple Texture struct.
 */

/**
 * @brief Delete the underlying OpenGL texture object.
 */
Texture::~Texture(){
    if(id){
        glDeleteTextures(1, &id);
        id = 0;
    }
}
