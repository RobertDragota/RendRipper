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
    void computeBounds();

    static constexpr glm::vec3 kModelColor   = glm::vec3(0.8f, 0.8f, 0.8f);
    static constexpr glm::vec3 kInfillColor  = glm::vec3(0.9f, 0.4f, 0.1f);
    static constexpr glm::vec3 kSupportColor = glm::vec3(0.1f, 0.5f, 0.9f);

    // lineVertices_ is no longer used directly; we bucket segments into layers_.
    // We keep bounds of ALL points (regardless of layer) so that a “layer slider” scaled correctly if needed.
    glm::vec3 center_;
    float radius_;

    struct ColoredVertex {
        glm::vec3 pos;
        glm::vec3 color;
    };

    // Each layer is now a flat list of ColoredVertex pairs forming line segments
    // layers_[i] = vertices for layer i, in consecutive pairs.
    std::vector<std::vector<ColoredVertex>> layers_;
    std::vector<float> layerZs_;

    // OpenGL handles: each layer gets its own VAO/VBO pair.
    std::vector<unsigned int> layerVAOs_;
    std::vector<unsigned int> layerVBOs_;

    bool ready_{false};

    // We do *not* keep one big “lineVertices_” vector anymore; it's now split per layer.
    // Temporary storage is used only during parsing.
};
