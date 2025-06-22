#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Colored vertex used when uploading G-code lines.
 */
struct GCodeColoredVertex
{
    glm::vec3 pos;   ///< 3D position of the vertex
    glm::vec3 color; ///< Vertex color
};

/**
 * @brief Utility for parsing .gcode files into drawable layers.
 */
struct GCodeMeta
{
    int print_time_seconds = 0;
    double filament_used_m = 0.0;
};

class GCodeParser
{
public:
    /**
     * @brief Parse the specified file into layered vertex data.
     * @param path Path to the .gcode file.
     * @param layers Output vertex list per layer.
     * @param layerZs Output Z height for each layer.
     */
    void Parse
    (
        const std::string &path,
        std::vector<std::vector<GCodeColoredVertex> > &layers,
        std::vector<float> &layerZs,
        GCodeMeta *meta = nullptr
    ) const;
};
