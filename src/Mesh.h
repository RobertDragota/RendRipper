#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"

struct Vertex { glm::vec3 pos, norm; glm::vec2 uv; };
struct Texture { unsigned int id; std::string type, path; };

class Mesh {
public:
    Mesh(const std::vector<Vertex>& verts,const std::vector<unsigned int>& idxs,const std::vector<Texture>& texs);
    void Draw(const Shader& shader) const;
private:
    unsigned int VAO,VBO,EBO;
    std::vector<Vertex> vertices;
public:
    const std::vector<Vertex> &getVertices() const;

private:
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    void setup();
};