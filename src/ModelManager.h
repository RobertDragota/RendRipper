#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"
#include "Transform.h"


class ModelManager {
public:
    int LoadModel(const std::string &modelPath);
    void UnloadModel(int index);

    /// Export the model with its current transform applied.
    /// If includeTranslation is false, the translation component is ignored so
    /// the mesh is exported around the origin. This is useful when the slicer
    /// will position the part via mesh_position_x/y.
    void ExportTransformedModel(int index, const std::string &outPath,
                               bool includeTranslation = true) const;

    size_t Count() const { return models_.size(); }
    Model* GetModel(int index) { return index>=0 && index<(int)models_.size()? models_[index].get():nullptr; }
    Shader* GetShader(int index) { return index>=0 && index<(int)shaders_.size()? shaders_[index].get():nullptr; }
    Transform* GetTransform(int index) { return index>=0 && index<(int)transforms_.size()? transforms_[index].get():nullptr; }
    glm::vec3 GetDimensions(int index) const { return meshDimensions_[index]; }
    const std::string &GetPath(int index) const { return modelPaths_[index]; }

    /// Return the world-space center of the model after transform
    glm::vec3 GetWorldCenter(int index) const;

    /// Check if the model fits within the given half-width/half-depth bed
    bool FitsInBed(int index, float bedHalfX, float bedHalfY) const;

    void EnforceGridConstraint(int index);
    void UpdateDimensions(int index);

private:
    std::vector<std::unique_ptr<Shader>> shaders_;
    std::vector<std::unique_ptr<Model>> models_;
    std::vector<std::unique_ptr<Transform>> transforms_;
    std::vector<glm::vec3> meshDimensions_;
    std::vector<std::string> modelPaths_;
};
