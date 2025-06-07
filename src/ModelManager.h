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

    void ExportTransformedModel(int index, const std::string &outPath) const;

    size_t Count() const { return models_.size(); }
    Model* GetModel(int index) { return index>=0 && index<(int)models_.size()? models_[index].get():nullptr; }
    Shader* GetShader(int index) { return index>=0 && index<(int)shaders_.size()? shaders_[index].get():nullptr; }
    Transform* GetTransform(int index) { return index>=0 && index<(int)transforms_.size()? transforms_[index].get():nullptr; }
    glm::vec3 GetDimensions(int index) const { return meshDimensions_[index]; }
    const std::string &GetPath(int index) const { return modelPaths_[index]; }

    void EnforceGridConstraint(int index);
    void UpdateDimensions(int index);

private:
    std::vector<std::unique_ptr<Shader>> shaders_;
    std::vector<std::unique_ptr<Model>> models_;
    std::vector<std::unique_ptr<Transform>> transforms_;
    std::vector<glm::vec3> meshDimensions_;
    std::vector<std::string> modelPaths_;
};
