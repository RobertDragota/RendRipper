#include "Mesh.h"
#include <glad/glad.h>

Texture::~Texture(){
    if(id){
        glDeleteTextures(1, &id);
        id = 0;
    }
}
