// GCodeModel.cpp
#include "GCodeModel.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <cfloat>   // for FLT_MAX

GCodeModel::GCodeModel(const std::string& gcodePath) {
    parseGCode(gcodePath);
    if (!layers_.empty()) {
        computeBounds();
        uploadToGPU();
        ready_ = true;
    } else {
        std::cerr << "Warning: GCodeModel loaded no extruding moves from " << gcodePath << std::endl;
    }
}

GCodeModel::~GCodeModel() {
    // Delete all VAOs and VBOs
    for (unsigned int vao : layerVAOs_) {
        if (vao) glDeleteVertexArrays(1, &vao);
    }
    for (unsigned int vbo : layerVBOs_) {
        if (vbo) glDeleteBuffers(1, &vbo);
    }
}

void GCodeModel::parseGCode(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open G-code file: " + path);
    }

    std::string line;
    glm::vec3 lastPos(0.0f);
    bool hasLastPos = false;
    float lastExtrusion = 0.0f;
    float currentZ = 0.0f;
    int currentLayerIndex = -1;

    // Look for either G0 or G1 commands (up until a ‘;’ comment)
    // We capture everything after the “G0 ” or “G1 ” prefix, ignoring trailing comments.
    std::regex moveRegex(R"(^(?:G0|G1)\s+([^;]*))");

    while (std::getline(in, line)) {
        std::smatch m;
        if (!std::regex_search(line, m, moveRegex)) {
            // Not a G0/G1 move or it's purely a comment
            continue;
        }

        // m[1] is the substring after “G0 ” or “G1 ” and before “;”
        std::string tokenStr = m[1].str();
        std::istringstream tokenStream(tokenStr);
        std::string token;

        // We'll try to parse up to four fields: X, Y, Z, and E
        float X = NAN, Y = NAN, Z = NAN, E = NAN;

        // Split on whitespace. Example tokens might be:
        //   "X30.646", "Y5.969", "E26.13131"
        while (tokenStream >> token) {
            if (token.size() < 2) {
                // Token too short (e.g. just "X" with no number), skip it
                continue;
            }

            char prefix = token[0];
            std::string numberPart = token.substr(1);  // everything after the letter

            // Attempt to convert numberPart to a float. If it fails, skip.
            float value = 0.0f;
            bool ok = false;
            try {
                value = std::stof(numberPart);
                ok = true;
            } catch (const std::exception&) {
                ok = false;
            }
            if (!ok) {
                // Non-numeric token (like E{...}), skip it
                continue;
            }

            switch (prefix) {
                case 'X': X = value; break;
                case 'Y': Y = value; break;
                case 'Z': Z = value; break;
                case 'E': E = value; break;
                default:
                    // Ignore F, M, S, etc.
                    break;
            }
        }

        // If Z has changed, start a brand‐new layer
        if (!std::isnan(Z) && Z != currentZ) {
            currentZ = Z;
            layerZs_.push_back(currentZ);
            layers_.emplace_back();
            currentLayerIndex = static_cast<int>(layers_.size()) - 1;
        }

        // Build currentPos from lastPos (so X, Y, Z only override if they were present)
        glm::vec3 currentPos = lastPos;
        if (!std::isnan(X)) currentPos.x = X;
        if (!std::isnan(Y)) currentPos.y = Y;
        if (!std::isnan(Z)) currentPos.z = Z;

        // If extrusion (E) increased and we have a valid lastPos, record a segment
        if (!std::isnan(E) && E > lastExtrusion && hasLastPos) {
            if (currentLayerIndex < 0) {
                // No Z encountered yet, but we see extrusion → force first layer
                currentLayerIndex = 0;
                layerZs_.push_back(currentPos.z);
                layers_.emplace_back();
            }
            // Add a pair of points (lastPos → currentPos) into layer currentLayerIndex
            layers_[currentLayerIndex].push_back(lastPos);
            layers_[currentLayerIndex].push_back(currentPos);
        }

        // Update lastExtrusion and lastPos for the next iteration
        if (!std::isnan(E)) {
            lastExtrusion = E;
        }
        lastPos = currentPos;
        hasLastPos = true;
    }
}

void GCodeModel::computeBounds() {
    // Compute center_ and radius_ over all layers
    glm::vec3 mn( FLT_MAX ), mx( -FLT_MAX );
    for (const auto& layerVerts : layers_) {
        for (const auto& v : layerVerts) {
            mn = glm::min(mn, v);
            mx = glm::max(mx, v);
        }
    }
    if (layers_.empty()) {
        center_ = glm::vec3(0.0f);
        radius_ = 0.0f;
        return;
    }
    center_ = (mn + mx) * 0.5f;
    radius_ = glm::length(mx - center_) * 0.5f;
}

void GCodeModel::uploadToGPU() {
    // For each layer i, create a VAO/VBO and upload layers_[i]'s vertex data
    size_t layerCount = layers_.size();
    layerVAOs_.resize(layerCount, 0);
    layerVBOs_.resize(layerCount, 0);

    for (size_t i = 0; i < layerCount; ++i) {
        const auto& verts = layers_[i];
        if (verts.empty())
            continue;
        unsigned int vao = 0, vbo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     verts.size() * sizeof(glm::vec3),
                     verts.data(),
                     GL_STATIC_DRAW);
        // Only position → location = 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        layerVAOs_[i] = vao;
        layerVBOs_[i] = vbo;
    }
}

// Draw a single layer index. Returns false if invalid index or not ready.
bool GCodeModel::DrawLayer(int layerIndex, Shader& lineShader) const {
    if (!ready_) return false;
    if (layerIndex < 0 || layerIndex >= static_cast<int>(layers_.size()))
        return false;
    const auto& verts = layers_[layerIndex];
    if (verts.empty())
        return false;

    lineShader.use();
    // We assume the caller already set “view” and “projection” uniforms.
    // If your shader has a uniform “lineColor”, you can set it here. E.g.:
    if (lineShader.hasUniform("lineColor")) {
        lineShader.setVec3("lineColor", glm::vec3(1.0f, 0.0f, 0.0f)); // default red for toolpaths
    }
    // Or pick a gradient color based on layerIndex, etc.

    glBindVertexArray(layerVAOs_[layerIndex]);
    // Each layer has exactly (layers_[i].size()/2) segments → so total vertices = layers_[i].size()
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(layers_[layerIndex].size()));
    glBindVertexArray(0);
    return true;
}

// Draw layers 0..maxLayerIndex inclusive. If maxLayerIndex < 0, draw all layers.
void GCodeModel::DrawUpToLayer(int maxLayerIndex, Shader& lineShader) const {
    if (!ready_) return;

    int layerCount = static_cast<int>(layers_.size());
    int end = (maxLayerIndex < 0 || maxLayerIndex >= layerCount) ? (layerCount - 1) : maxLayerIndex;

    lineShader.use();
    if (lineShader.hasUniform("lineColor")) {
        // For example, draw all printed lines in a dimmer red:
        lineShader.setVec3("lineColor", glm::vec3(1.0f, 0.3f, 0.3f));
    }

    for (int i = 0; i <= end; ++i) {
        const auto& verts = layers_[i];
        if (verts.empty()) continue;
        glBindVertexArray(layerVAOs_[i]);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size()));
    }
    glBindVertexArray(0);
}
