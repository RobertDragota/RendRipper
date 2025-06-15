#pragma once
#include <vector>
#include "GCodeParser.h"
#include <glad/glad.h>

/**
 * @brief Helper for transferring G-code vertices to OpenGL buffers.
 */
class GCodeUploader
{
public:
    /**
     * @brief Upload the vertex data for each layer.
     * @param layers Layered vertices parsed from a G-code file.
     * @param vaos Output array of created VAO handles.
     * @param vbos Output array of created VBO handles.
     */
    void Upload
    (
        const std::vector<std::vector<GCodeColoredVertex> > &layers,
        std::vector<unsigned int> &vaos,
        std::vector<unsigned int> &vbos
    ) const;
};
