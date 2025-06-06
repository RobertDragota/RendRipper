#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Mesh.h"
#include "ModelLoader.h"
enum class Axis { X_UP, Y_UP, Z_UP };
class Model {
public:
    explicit Model( std::string& path);
    ~Model();


    // movable
    Model(Model&&) noexcept;
    Model& operator=(Model&&) noexcept;

    void Draw(const Shader& shader) const;

    // bounding info
    glm::vec3 center;
    float radius;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    Axis determineUpAxis() const;

    std::vector<glm::vec3> getAllPositions() const;


    const std::vector<Mesh>& getMeshes() const noexcept { return meshes; }

    void computeBounds();

private:
    std::string directory;
    std::vector<Mesh> meshes;

};
