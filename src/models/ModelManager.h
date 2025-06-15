#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"
#include "Transform.h"


/**
 * @brief Maintains a collection of loaded 3D models and their transforms.
 */
class ModelManager {
public:
    /**
     * @brief Load a model from disk and create associated resources.
     * @return Index of the newly loaded model.
     */
    int LoadModel(const std::string &modelPath);

    /**
     * @brief Remove a model and its resources by index.
     */
    void UnloadModel(int index);

    /**
     * @brief Export the transformed mesh data to a new STL file.
     */
    void ExportTransformedModel(int index, const std::string &outPath) const;

    /** @brief Number of currently loaded models. */
    size_t Count() const { return models_.size(); }

    /** @brief Get pointer to model at index or nullptr. */
    Model* GetModel(int index) { return index>=0 && index<(int)models_.size()? models_[index].get():nullptr; }

    /** @brief Get shader associated with model index. */
    Shader* GetShader(int index) { return index>=0 && index<(int)shaders_.size()? shaders_[index].get():nullptr; }

    /** @brief Access the transform of a model. */
    Transform* GetTransform(int index) { return index>=0 && index<(int)transforms_.size()? transforms_[index].get():nullptr; }

    /** @brief Pre-computed mesh dimensions after scaling. */
    glm::vec3 GetDimensions(int index) const { return meshDimensions_[index]; }

    /** @brief Path to the STL file used for the model. */
    const std::string &GetPath(int index) const { return modelPaths_[index]; }

    /**
     * @brief Adjust a model so it rests on the ground plane.
     */
    void EnforceGridConstraint(int index);

    /**
     * @brief Recalculate the current scaled dimensions of a model.
     */
    void UpdateDimensions(int index);

private:
    std::vector<std::shared_ptr<Shader>> shaders_;
    std::vector<std::unique_ptr<Model>> models_;
    std::vector<std::unique_ptr<Transform>> transforms_;
    std::vector<glm::vec3> meshDimensions_;
    std::vector<std::string> modelPaths_;
};
