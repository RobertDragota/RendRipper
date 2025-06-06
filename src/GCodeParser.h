#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct GCodeColoredVertex {
    glm::vec3 pos;
    glm::vec3 color;
};

class GCodeParser {
public:
    void Parse(const std::string &path,
               std::vector<std::vector<GCodeColoredVertex>> &layers,
               std::vector<float> &layerZs) const;
};
