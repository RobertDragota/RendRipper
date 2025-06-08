#include "Mesh.h"
#include <glad/glad.h>

Mesh::Mesh(std::vector<Vertex> v, std::vector<unsigned> i, std::vector<std::shared_ptr<Texture>> t)
        : vertices(std::move(v)), indices(std::move(i)), textures(std::move(t)),
          VAO(0), VBO(0), EBO(0)
{
    setup();
}

Mesh::~Mesh() {

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

}

Mesh::Mesh(Mesh&& o) noexcept
        : vertices(std::move(o.vertices)), indices(std::move(o.indices)),
          textures(std::move(o.textures)), VAO(o.VAO), VBO(o.VBO), EBO(o.EBO)
{
    o.VAO = o.VBO = o.EBO = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {

        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);


        vertices = std::move(o.vertices);
        indices  = std::move(o.indices);
        textures = std::move(o.textures);
        VAO = o.VAO; VBO = o.VBO; EBO = o.EBO;
        o.VAO = o.VBO = o.EBO = 0;
    }
    return *this;
}

void Mesh::setup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    // norm
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,norm));
    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));

    glBindVertexArray(0);
}

void Mesh::Draw(const Shader& shader) const {
    unsigned int diffuseNr = 1, specularNr = 1;
    for (unsigned i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        if (textures[i]->type == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else
            number = std::to_string(specularNr++);
        shader.setInt(textures[i]->type + number, i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->id);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
