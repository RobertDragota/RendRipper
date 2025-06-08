#include "Model.h"
#include "Shader.h"
#include "glad/glad.h"
#include "ObjConverter.h"
#include "TextureLoader.h"
#include "ModelLoader.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

Model::Model( std::string& path) {
    directory = path.substr(0, path.find_last_of("/\\"));

    std::cout<< "Loading model from: " << path << std::endl;
    std::string outPath = directory + "/temp.stl";
    ObjConverter::Convert(path, outPath);
    ModelLoader loader;
    meshes = loader.Load(path);
    computeBounds();

}

Model::~Model() = default;

Model::Model(Model&& o) noexcept
        : directory(std::move(o.directory)), meshes(std::move(o.meshes)),
          center(o.center), radius(o.radius)
{}

Model& Model::operator=(Model&& o) noexcept {
    if (this != &o) {
        directory = std::move(o.directory);
        meshes = std::move(o.meshes);
        center = o.center;
        radius = o.radius;
    }
    return *this;
}

void Model::Draw(const Shader& shader) const {
    for (auto& m : meshes)
        m.Draw(shader);
}





void Model::computeBounds() {
    glm::vec3 mn( FLT_MAX), mx(-FLT_MAX);
    for (auto& mesh : meshes)
        for (auto& v : mesh.getVertices()) {
            mn = glm::min(mn, v.pos);
            mx = glm::max(mx, v.pos);
        }
    center = (mn + mx) * 0.5f;
    radius = glm::length(mx - center);
    minBounds = mn;
    maxBounds = mx;
    center = (mn + mx) * 0.5f;
    radius = glm::length(mx - mn) * 0.5f;
}


Axis Model::determineUpAxis() const {
    // Bounding box analysis
    glm::vec3 extents = maxBounds - minBounds;
    int mainExtentAxis = 0; // X=0, Y=1, Z=2
    if (extents.y > extents.x && extents.y > extents.z) {
        mainExtentAxis = 1;
    } else if (extents.z > extents.x) {
        mainExtentAxis = 2;
    }

    // Normal analysis
    std::vector<glm::vec3> allNormals;
    for (const auto& mesh : meshes) {
        const auto& vertices = mesh.getVertices();
        for (const auto& vertex : vertices) {
            allNormals.push_back(vertex.norm);
        }
    }

    const float threshold = glm::cos(glm::radians(10.0f)); // 10-degree threshold
    float normalScores[3] = {0.0f, 0.0f, 0.0f}; // X, Y, Z

    for (const auto& norm : allNormals) {
        for (int axis = 0; axis < 3; ++axis) {
            glm::vec3 axisDir(0.0f);
            axisDir[axis] = -1.0f; // Check negative axis alignment
            float dot = glm::dot(glm::normalize(norm), axisDir);
            if (dot >= threshold) {
                normalScores[axis] += 1.0f;
            }
        }
    }

    // Normalize normal scores
    if (!allNormals.empty()) {
        for (float& score : normalScores) {
            score /= allNormals.size();
        }
    }

    // Vertex distribution analysis
    std::vector<glm::vec3> allVerts;
    for (const auto& mesh : meshes) {
        const auto& vertices = mesh.getVertices();
        for (const auto& vertex : vertices) {
            allVerts.push_back(vertex.pos);
        }
    }

    const float clusterThreshold = 0.1f; // 10% of extent
    float clusterScores[3] = {0.0f, 0.0f, 0.0f}; // X, Y, Z

    for (int axis = 0; axis < 3; ++axis) {
        const float minVal = minBounds[axis];
        const float clusterMax = minVal + extents[axis] * clusterThreshold;
        for (const auto& pos : allVerts) {
            if (pos[axis] >= minVal && pos[axis] <= clusterMax) {
                clusterScores[axis] += 1.0f;
            }
        }
    }

    // Normalize cluster scores
    if (!allVerts.empty()) {
        for (float& score : clusterScores) {
            score /= allVerts.size();
        }
    }

    // Combine scores with weights
    const float bboxWeight = 0.4f;
    const float normalWeight = 0.4f;
    const float clusterWeight = 0.2f;

    float finalScores[3] = {0.0f};
    finalScores[mainExtentAxis] += bboxWeight;
    for (int axis = 0; axis < 3; ++axis) {
        finalScores[axis] += normalScores[axis] * normalWeight;
        finalScores[axis] += clusterScores[axis] * clusterWeight;
    }

    // Determine the axis with the highest score
    int maxAxis = 0;
    for (int axis = 1; axis < 3; ++axis) {
        if (finalScores[axis] > finalScores[maxAxis]) {
            maxAxis = axis;
        }
    }

    return static_cast<Axis>(maxAxis);
}

std::vector<glm::vec3> Model::getAllPositions() const {
    std::vector<glm::vec3> out;
    // Reserve approximate total size for performance
    size_t total = 0;
    for (const auto &m : meshes) {
        total += m.getVertices().size();
    }
    out.reserve(total);

    // Gather all vertex positions
    for (const auto &mesh : meshes) {
        for (const auto &v : mesh.getVertices()) {
            out.push_back(v.pos);
        }
    }
    return out;
}

glm::vec3 Model::computeMassCenter() const {
    glm::vec3 sum(0.0f);
    size_t count = 0;
    for (const auto& mesh : meshes) {
        for (const auto& v : mesh.getVertices()) {
            sum += v.pos;
            ++count;
        }
    }
    if (count == 0) return glm::vec3(0.0f);
    return sum / static_cast<float>(count);
}
