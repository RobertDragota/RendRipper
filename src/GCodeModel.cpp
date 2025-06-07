// GCodeModel.cpp
#include "GCodeModel.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <cfloat>   // for FLT_MAX
#include <algorithm>
#include <cmath>
#include "GCodeParser.h"
#include "GCodeUploader.h"

GCodeModel::GCodeModel(const std::string& gcodePath) {
    GCodeParser parser;
    parser.Parse(gcodePath, layers_, layerZs_);
    if (!layers_.empty()) {
        computeBounds();
        GCodeUploader uploader;
        uploader.Upload(layers_, layerVAOs_, layerVBOs_);
        layerVertexCounts_.reserve(layers_.size());
        for (const auto& layer : layers_) {
            layerVertexCounts_.push_back(layer.size());
        }
        std::vector<std::vector<ColoredVertex>>().swap(layers_);
        layerZs_.shrink_to_fit();
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


void GCodeModel::computeBounds() {
    // Compute center_ and radius_ over all layers
    glm::vec3 mn( FLT_MAX ), mx( -FLT_MAX );
    for (const auto& layerVerts : layers_) {
        for (const auto& v : layerVerts) {
            mn = glm::min(mn, v.pos);
            mx = glm::max(mx, v.pos);
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


// Draw a single layer index. Returns false if invalid index or not ready.
bool GCodeModel::DrawLayer(int layerIndex, Shader& lineShader) const {
    if (!ready_) return false;
    if (layerIndex < 0 || layerIndex >= static_cast<int>(layerVertexCounts_.size()))
        return false;
    if (layerVertexCounts_[layerIndex] == 0)
        return false;

    lineShader.use();
    // We assume the caller already set “view” and “projection” uniforms.
    // If your shader has a uniform “lineColor”, you can set it here. E.g.:
    if (lineShader.hasUniform("lineColor")) {
        lineShader.setVec3("lineColor", glm::vec3(1.0f, 0.0f, 0.0f)); // default red for toolpaths
    }
    // Or pick a gradient color based on layerIndex, etc.

    glBindVertexArray(layerVAOs_[layerIndex]);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(layerVertexCounts_[layerIndex]));
    glLineWidth(1.0f);
    glBindVertexArray(0);
    return true;
}

// Draw layers 0..maxLayerIndex inclusive. If maxLayerIndex < 0, draw all layers.
void GCodeModel::DrawUpToLayer(int maxLayerIndex, Shader& lineShader) const {
    if (!ready_) return;

    int layerCount = static_cast<int>(layerVertexCounts_.size());
    int end = (maxLayerIndex < 0 || maxLayerIndex >= layerCount) ? (layerCount - 1) : maxLayerIndex;

    lineShader.use();
    if (lineShader.hasUniform("lineColor")) {
        // For example, draw all printed lines in a dimmer red:
        lineShader.setVec3("lineColor", glm::vec3(1.0f, 0.3f, 0.3f));
    }

    for (int i = 0; i <= end; ++i) {
        if (layerVertexCounts_[i] == 0) continue;
        glBindVertexArray(layerVAOs_[i]);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(layerVertexCounts_[i]));
        glLineWidth(1.0f);
    }
    glBindVertexArray(0);
}
