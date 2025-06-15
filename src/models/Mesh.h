#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Shader.h"

/**
 * @brief Basic vertex description used by Mesh.
 */
struct Vertex {
    glm::vec3 pos;  ///< Vertex position
    glm::vec3 norm; ///< Vertex normal
    glm::vec2 uv;   ///< Texture coordinates
};

/**
 * @brief Simple texture container used when loading models.
 */
struct Texture {
    unsigned int id = 0; ///< OpenGL texture object id
    std::string type;    ///< Diffuse/specular/etc
    std::string path;    ///< Source path

    ~Texture();
};

/**
 * @brief Container for vertex/index/texture data along with its VAO.
 */
class Mesh {
public:
    /**
     * @brief Construct from raw vertex/index/texture arrays.
     */
    Mesh(std::vector<Vertex> verts, std::vector<unsigned> idxs, std::vector<std::shared_ptr<Texture>> texs);

    /** @brief Destroy OpenGL buffers. */
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    /// Move constructor
    Mesh(Mesh&&) noexcept;

    /// Move assignment operator
    Mesh& operator=(Mesh&&) noexcept;

    /**
     * @brief Render the mesh using the supplied shader.
     */
    void Draw(const Shader& shader) const;
    [[nodiscard]] const std::vector<Vertex>& getVertices() const noexcept { return vertices; }
    [[nodiscard]] const std::vector<unsigned>& getIndices()  const noexcept { return indices; }


private:
    /**
     * @brief Create OpenGL buffers and upload mesh data.
     */
    void setup();

    std::vector<Vertex>  vertices;
    std::vector<unsigned> indices;
    std::vector<std::shared_ptr<Texture>> textures;
    unsigned int VAO, VBO, EBO;
};
