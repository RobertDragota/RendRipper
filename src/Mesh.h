#pragma once
#include <vector>
#include <string>
#include "Shader.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 uv;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    Mesh(std::vector<Vertex> verts, std::vector<unsigned> idxs, std::vector<Texture> texs);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;
    Mesh& operator=(Mesh&&) noexcept;

    void Draw(const Shader& shader) const;
    const std::vector<Vertex>& getVertices() const noexcept { return vertices; }
    const std::vector<unsigned>& getIndices()  const noexcept { return indices; }


private:
    void setup();

    std::vector<Vertex>  vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;
    unsigned int VAO, VBO, EBO;
};
