#include "Mesh.h"
#include <glad/glad.h>

Mesh::Mesh(const std::vector<Vertex>& v,const std::vector<unsigned int>& i,const std::vector<Texture>& t)
        :vertices(v),indices(i),textures(t){ setup(); }
void Mesh::setup(){
    glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO); glGenBuffers(1,&EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),indices.data(),GL_STATIC_DRAW);
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    // norm
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,norm));
    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glBindVertexArray(0);
}
void Mesh::Draw(const Shader& shader) const{
    unsigned int di=1,si=1;
    for(unsigned i=0;i<textures.size();++i){
        glActiveTexture(GL_TEXTURE0+i);
        std::string num = (textures[i].type=="texture_diffuse"? std::to_string(di++): std::to_string(si++));
        shader.setInt(textures[i].type+num,i);
        glBindTexture(GL_TEXTURE_2D,textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

const std::vector<Vertex> &Mesh::getVertices() const {
    return vertices;
}
