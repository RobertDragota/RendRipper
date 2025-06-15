#include "ModelManager.h"
#include <algorithm>
#include <vector>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <memory>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


#include "MeshRepairer.h"
#include "ShaderCache.h"

/**
 * @brief Load a model from an STL file and create default shader/transform.
 */
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
    shaders_.emplace_back(
        ShaderCache::Get("../../resources/shaders/model_shader.vert",
                        "../../resources/shaders/model_shader.frag"));
    models_.emplace_back(std::move(mPtr));
    int idx = static_cast<int>(models_.size()) - 1;
    EnforceGridConstraint(idx);
    return idx;
}

/** @brief Remove model and associated resources from the manager. */
void ModelManager::UnloadModel(int index) {
    if (index < 0 || index >= static_cast<int>(models_.size())) return;
    models_.erase(models_.begin() + index);
    shaders_.erase(shaders_.begin() + index);
    transforms_.erase(transforms_.begin() + index);
    meshDimensions_.erase(meshDimensions_.begin() + index);
    modelPaths_.erase(modelPaths_.begin() + index);
}

/**
 * @brief Ensure a model sits on the ground plane after transformations.
 */
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

/** @brief Update cached dimensions after a transform change. */
void ModelManager::UpdateDimensions(int index) {
    if (index < 0 || index >= static_cast<int>(models_.size())) return;
    glm::vec3 base = models_[index]->maxBounds - models_[index]->minBounds;
    glm::vec3 sc = transforms_[index]->scale;
    meshDimensions_[index] = base * sc;
}


/**
 * @brief Export the model with its current transform applied into a new file.
 */
void ModelManager::ExportTransformedModel(int index, const std::string &outPath) const {

    if (index < 0 || index >= static_cast<int>(models_.size())) return;

    const Model &model = *models_[index];
    const Transform &tf = *transforms_[index];

    glm::mat4 mat = tf.getMatrix();

    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mat)));

    auto scene = std::make_unique<aiScene>();
    scene->mRootNode = new aiNode();
    scene->mNumMeshes = static_cast<unsigned int>(model.getMeshes().size());
    scene->mMeshes = new aiMesh*[scene->mNumMeshes];
    scene->mRootNode->mNumMeshes = scene->mNumMeshes;
    scene->mRootNode->mMeshes = new unsigned int[scene->mNumMeshes];
    scene->mNumMaterials = 1;
    scene->mMaterials = new aiMaterial*[1];
    scene->mMaterials[0] = new aiMaterial();

    for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi) {
        const auto &src = model.getMeshes()[mi];
        const auto &verts = src.getVertices();
        const auto &idxs = src.getIndices();

        aiMesh *mesh = new aiMesh();
        mesh->mMaterialIndex = 0;
        mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
        mesh->mNumVertices = static_cast<unsigned int>(verts.size());
        mesh->mVertices = new aiVector3D[mesh->mNumVertices];
        mesh->mNormals = new aiVector3D[mesh->mNumVertices];

        for (unsigned int vi = 0; vi < mesh->mNumVertices; ++vi) {
            glm::vec3 pos = glm::vec3(mat * glm::vec4(verts[vi].pos, 1.0f));
            glm::vec3 nrm = glm::normalize(normalMat * verts[vi].norm);
            mesh->mVertices[vi] = aiVector3D(pos.x, pos.y, pos.z);
            mesh->mNormals[vi] = aiVector3D(nrm.x, nrm.y, nrm.z);
        }

        mesh->mNumFaces = static_cast<unsigned int>(idxs.size() / 3);
        mesh->mFaces = new aiFace[mesh->mNumFaces];
        for (unsigned int fi = 0; fi < mesh->mNumFaces; ++fi) {
            aiFace &face = mesh->mFaces[fi];
            face.mNumIndices = 3;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = idxs[fi * 3];
            face.mIndices[1] = idxs[fi * 3 + 1];
            face.mIndices[2] = idxs[fi * 3 + 2];
        }

        scene->mMeshes[mi] = mesh;
        scene->mRootNode->mMeshes[mi] = mi;
    }

    Assimp::Exporter exporter;
    aiReturn ret = exporter.Export(scene.get(), "stl", outPath);
    if (ret != aiReturn_SUCCESS) {
        throw std::runtime_error(std::string("Assimp export error: ") + exporter.GetErrorString());
    }
}

