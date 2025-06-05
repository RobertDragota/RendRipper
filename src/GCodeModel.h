// GCodeModel.h
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <glm/glm.hpp>
#include "Shader.h"

/// GCodeModel now groups extruding moves by “layer” (unique Z values).
/// Each layer is one sequence of GL_LINES. You can draw a single layer or all layers up to some index.
class GCodeModel {
public:
    /// Constructor: parse the .gcode file immediately.
    explicit GCodeModel(const std::string& gcodePath);
    ~GCodeModel();

    /// Draw *only* layer 'layerIndex' (0-based).
    /// Returns false if layerIndex is invalid.
    bool DrawLayer(int layerIndex, Shader& lineShader) const;

    /// Draw all layers from 0..maxLayerIndex inclusive.
    /// If maxLayerIndex < 0, draws all layers.
    void DrawUpToLayer(int maxLayerIndex, Shader& lineShader) const;

    /// Number of layers parsed
    int GetLayerCount() const { return static_cast<int>(layers_.size()); }

    /// Returns the Z-height (in mm) of each layer index.
    /// That is, layerZs_[i] = the Z coordinate that was first encountered for layer i.
    const std::vector<float>& GetLayerHeights() const { return layerZs_; }

private:
    void parseGCode(const std::string& path);
    void computeBounds();
    void uploadToGPU();

    // lineVertices_ is no longer used directly; we bucket segments into layers_.
    // We keep bounds of ALL points (regardless of layer) so that a “layer slider” scaled correctly if needed.
    glm::vec3 center_;
    float radius_;

    // Each layer = a flat list of glm::vec3 (pairs = line segments).
    // layers_[i] = vertices for layer i, in consecutive pairs.
    std::vector<std::vector<glm::vec3>> layers_;
    std::vector<float> layerZs_;

    // OpenGL handles: each layer gets its own VAO/VBO pair.
    std::vector<unsigned int> layerVAOs_;
    std::vector<unsigned int> layerVBOs_;

    bool ready_{false};

    // We do *not* keep one big “lineVertices_” vector anymore; it's now split per layer.
    // We only use temporary storage in parseGCode.
};
