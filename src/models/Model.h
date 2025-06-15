#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Mesh.h"
#include "ModelLoader.h"
enum class Axis { X_UP, Y_UP, Z_UP };
/**
 * @brief In-memory representation of a loaded 3D model.
 *
 * A Model owns a collection of Mesh objects loaded from disk and
 * provides helper functions for rendering and computing bounds.
 */
class Model {
public:
    /**
     * @brief Load a model from the given file path.
     * @param path Path to the source model file.
     */
    explicit Model(std::string& path);

    /** @brief Destroy the model and release resources. */
    ~Model();


    // movable
    /** @brief Move constructor. */
    Model(Model&&) noexcept;

    /** @brief Move assignment operator. */
    Model& operator=(Model&&) noexcept;

    /**
     * @brief Render all meshes using the supplied shader.
     */
    void Draw(const Shader& shader) const;

    // bounding info
    glm::vec3 center;
    float radius;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    /**
     * @brief Guess the model's up axis based on geometry.
     */
    Axis determineUpAxis() const;

    /**
     * @brief Collect positions of every vertex across all meshes.
     */
    std::vector<glm::vec3> getAllPositions() const;
    /// Compute an approximate mass center by averaging all vertex positions
    /**
     * @brief Approximate the mass center by averaging all vertex positions.
     */
    glm::vec3 computeMassCenter() const;


    /** @brief Access the underlying mesh list. */
    const std::vector<Mesh>& getMeshes() const noexcept { return meshes; }

    /** @brief Recalculate the bounding box and radius for the model. */
    void computeBounds();

private:
    std::string directory;
    std::vector<Mesh> meshes;

};
