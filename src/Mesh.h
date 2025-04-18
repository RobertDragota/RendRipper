#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"

struct Vertex { glm::vec3 Position, Normal; glm::vec2 TexCoord; };
struct Texture { unsigned int id; std::string type, path; };

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);
private:
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    void setupMesh();
};
