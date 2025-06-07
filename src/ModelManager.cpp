#include "ModelManager.h"
#include <algorithm>
#include <fstream>
#include <vector>

#include "MeshRepairer.h"

int ModelManager::LoadModel(const std::string &modelPath) {
    std::string directory = modelPath.substr(0, modelPath.find_last_of("/\\"));
    std::string output = directory + "/output.stl";
    MeshRepairer::repairSTLFile(modelPath, output);
    auto mPtr = std::make_unique<Model>(output);
    mPtr->computeBounds();
    glm::vec3 size = mPtr->maxBounds - mPtr->minBounds;

    float maxDim = glm::max(size.x, glm::max(size.y, size.z));
    float scaleFactor = 1.0f;
    if (maxDim > 0.01f && maxDim <= 10.0f)
        {
        size = glm::normalize(size);
        scaleFactor = 100.0f;
        }

    size *= scaleFactor;
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
    t->translation = glm::vec3(-scaleFactor * c.x,
                               -scaleFactor * c.y,
                               -scaleFactor * minZ);
    t->scale = glm::vec3(scaleFactor);
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

void ModelManager::UpdateDimensions(int index) {
    if (index < 0 || index >= static_cast<int>(models_.size())) return;
    glm::vec3 base = models_[index]->maxBounds - models_[index]->minBounds;
    glm::vec3 sc = transforms_[index]->scale;
    meshDimensions_[index] = base * sc;
}

void ModelManager::ExportTransformedModel(int index, const std::string &outPath) const {
    if (index < 0 || index >= static_cast<int>(models_.size())) return;
    const Model &model = *models_[index];
    const Transform &tf = *transforms_[index];
    glm::mat4 mat = tf.getMatrix();

    struct Tri { glm::vec3 v0, v1, v2; };
    std::vector<Tri> tris;
    for (const auto &mesh : model.getMeshes()) {
        const auto &verts = mesh.getVertices();
        const auto &idxs = mesh.getIndices();
        for (size_t i = 0; i + 2 < idxs.size(); i += 3) {
            glm::vec3 p0 = glm::vec3(mat * glm::vec4(verts[idxs[i]].pos, 1.f));
            glm::vec3 p1 = glm::vec3(mat * glm::vec4(verts[idxs[i+1]].pos, 1.f));
            glm::vec3 p2 = glm::vec3(mat * glm::vec4(verts[idxs[i+2]].pos, 1.f));
            tris.push_back({p0, p1, p2});
        }
    }

    std::ofstream out(outPath, std::ios::binary);
    char header[80] = {0};
    out.write(header, 80);
    uint32_t count = static_cast<uint32_t>(tris.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    for (const auto &t : tris) {
        glm::vec3 normal = glm::normalize(glm::cross(t.v1 - t.v0, t.v2 - t.v0));
        out.write(reinterpret_cast<const char*>(&normal), sizeof(glm::vec3));
        out.write(reinterpret_cast<const char*>(&t.v0), sizeof(glm::vec3));
        out.write(reinterpret_cast<const char*>(&t.v1), sizeof(glm::vec3));
        out.write(reinterpret_cast<const char*>(&t.v2), sizeof(glm::vec3));
        uint16_t attr = 0;
        out.write(reinterpret_cast<const char*>(&attr), sizeof(uint16_t));
    }
}
