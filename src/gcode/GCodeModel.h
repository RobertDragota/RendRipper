// GCodeModel.h
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <glm/glm.hpp>
#include "Shader.h"
#include "GCodeParser.h" // for GCodeColoredVertex

/**
 * @brief Representation of a parsed G-code tool path.
 *
 * The model groups all extrusion moves into layers based on their Z
 * height. Each layer can be drawn individually or up to a specific
 * index for preview purposes.
 */
class GCodeModel
{
public:
    /**
     * @brief Parse the provided G-code file on construction.
     * @param gcodePath Path to the .gcode file on disk.
     */
    explicit GCodeModel(const std::string &gcodePath);

    /** @brief Release all GPU resources. */
    ~GCodeModel();

    /**
     * @brief Draw a single layer by index.
     * @param layerIndex Zero-based layer index to draw.
     * @param lineShader Shader used for rendering the lines.
     * @return False if the index is invalid.
     */
    bool DrawLayer(int layerIndex, Shader &lineShader) const;

    /**
     * @brief Draw all layers up to the specified index.
     * @param maxLayerIndex Inclusive maximum layer index, or negative for all.
     * @param lineShader Shader used for rendering.
     */
    void DrawUpToLayer(int maxLayerIndex, Shader &lineShader) const;

    /** @brief Number of parsed layers. */
    int GetLayerCount() const { return static_cast<int>(layerVertexCounts_.size()); }

    /**
     * @brief Z height for every parsed layer.
     * @return Array of layer heights.
     */
    const std::vector<float> &GetLayerHeights() const { return layerZs_; }

    /** @brief Minimum world bounds of the tool path. */
    const glm::vec3 &GetBoundsMin() const { return boundsMin_; }
    /** @brief Maximum world bounds of the tool path. */
    const glm::vec3 &GetBoundsMax() const { return boundsMax_; }
    /** @brief Center of all parsed vertices. */
    const glm::vec3 &GetCenter() const { return center_; }

private:
    /**
     * @brief Recalculate center, radius and bounds across all layers.
     *
     * Called once after parsing to compute the model's overall extents.
     */
    void computeBounds();

    static constexpr glm::vec3 kModelColor = glm::vec3(0.8f, 0.8f, 0.8f);
    static constexpr glm::vec3 kInfillColor = glm::vec3(0.9f, 0.4f, 0.1f);
    static constexpr glm::vec3 kSupportColor = glm::vec3(0.1f, 0.5f, 0.9f);

    // lineVertices_ is no longer used directly; we bucket segments into layers_.
    // We keep bounds of ALL points (regardless of layer) so that a “layer slider” scaled correctly if needed.
    glm::vec3 center_;
    float radius_;
    glm::vec3 boundsMin_;
    glm::vec3 boundsMax_;

    using ColoredVertex = GCodeColoredVertex;

    // Each layer is now a flat list of ColoredVertex pairs forming line segments
    // layers_[i] = vertices for layer i, in consecutive pairs.
    std::vector<std::vector<ColoredVertex> > layers_;
    std::vector<size_t> layerVertexCounts_;
    std::vector<float> layerZs_;

    // OpenGL handles: each layer gets its own VAO/VBO pair.
    std::vector<unsigned int> layerVAOs_;
    std::vector<unsigned int> layerVBOs_;

    bool ready_{false};

    // We do *not* keep one big “lineVertices_” vector anymore; it's now split per layer.
    // Temporary storage is used only during parsing.
};
