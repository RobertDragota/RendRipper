#include "ModelManager.h"
#include <algorithm>

int ModelManager::LoadModel(const std::string &modelPath) {
    std::string directory = modelPath.substr(0, modelPath.find_last_of("/\\"));
    std::string output = directory + "/output.stl";
    MeshRepairer::repairSTLFile(modelPath, output);
    auto mPtr = std::make_unique<Model>(output);
    mPtr->computeBounds();
    glm::vec3 size = mPtr->maxBounds - mPtr->minBounds;
    meshDimensions_.push_back(size);
    modelPaths_.push_back(output);

    auto t = std::make_unique<Transform>();
    glm::vec3 corners[8] = {
            {mPtr->minBounds.x, mPtr->minBounds.y, mPtr->minBounds.z},
            {mPtr->maxBounds.x, mPtr->minBounds.y, mPtr->minBounds.z},
            {mPtr->minBounds.x, mPtr->maxBounds.y, mPtr->minBounds.z},
            {mPtr->maxBounds.x, mPtr->maxBounds.y, mPtr->minBounds.z},
            {mPtr->minBounds.x, mPtr->minBounds.y, mPtr->maxBounds.z},
            {mPtr->maxBounds.x, mPtr->minBounds.y, mPtr->maxBounds.z},
            {mPtr->minBounds.x, mPtr->maxBounds.y, mPtr->maxBounds.z},
            {mPtr->maxBounds.x, mPtr->maxBounds.y, mPtr->maxBounds.z}
    };
    float minZ = corners[0].z;
    for (int i = 1; i < 8; ++i) minZ = std::min(minZ, corners[i].z);
    glm::vec3 c = mPtr->center;
    t->translation = glm::vec3(-c.x, -c.y, -minZ);
    t->scale = glm::vec3(1.0f);
    t->rotationQuat = glm::quat(1,0,0,0);
    transforms_.push_back(std::move(t));
    shaders_.emplace_back(std::make_unique<Shader>("../../resources/shaders/model_shader.vert",
                                                  "../../resources/shaders/model_shader.frag"));
    models_.emplace_back(std::move(mPtr));
    int idx = static_cast<int>(models_.size()) - 1;
    EnforceGridConstraint(idx);
    return idx;
}

void ModelManager::UnloadModel(int index) {
    if (index < 0 || index >= static_cast<int>(models_.size())) return;
    models_.erase(models_.begin() + index);
    shaders_.erase(shaders_.begin() + index);
    transforms_.erase(transforms_.begin() + index);
    meshDimensions_.erase(meshDimensions_.begin() + index);
    modelPaths_.erase(modelPaths_.begin() + index);
}

void ModelManager::EnforceGridConstraint(int index) {
    if (index < 0 || index >= static_cast<int>(models_.size())) return;
    Model &model = *models_[index];
    Transform &transform = *transforms_[index];
    glm::vec3 localCorners[8] = {
            {model.minBounds.x, model.minBounds.y, model.minBounds.z},
            {model.maxBounds.x, model.minBounds.y, model.minBounds.z},
            {model.minBounds.x, model.maxBounds.y, model.minBounds.z},
            {model.maxBounds.x, model.maxBounds.y, model.minBounds.z},
            {model.minBounds.x, model.minBounds.y, model.maxBounds.z},
            {model.maxBounds.x, model.minBounds.y, model.maxBounds.z},
            {model.minBounds.x, model.maxBounds.y, model.maxBounds.z},
            {model.maxBounds.x, model.maxBounds.y, model.maxBounds.z}
    };
    glm::mat4 modelMatrix = transform.getMatrix();
    float minZ = std::numeric_limits<float>::max();
    for (auto &lc : localCorners) {
        glm::vec3 worldCorner = glm::vec3(modelMatrix * glm::vec4(lc, 1.0f));
        minZ = glm::min(minZ, worldCorner.z);
    }
    float groundPlaneZ = 0.f;
    if (minZ < groundPlaneZ) {
        float offsetNeeded = groundPlaneZ - minZ;
        transform.translation.z += offsetNeeded;
    }
}
