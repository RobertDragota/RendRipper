#pragma once
#include <vector>
#include "GCodeParser.h"
#include <glad/glad.h>

class GCodeUploader {
public:
    void Upload(const std::vector<std::vector<GCodeColoredVertex>> &layers,
                std::vector<unsigned int> &vaos,
                std::vector<unsigned int> &vbos) const;
};
