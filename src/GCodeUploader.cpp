#include "GCodeUploader.h"

void GCodeUploader::Upload(const std::vector<std::vector<GCodeColoredVertex>> &layers,
                           std::vector<unsigned int> &vaos,
                           std::vector<unsigned int> &vbos) const
{
    size_t layerCount = layers.size();
    vaos.resize(layerCount, 0);
    vbos.resize(layerCount, 0);

    for (size_t i = 0; i < layerCount; ++i) {
        const auto &verts = layers[i];
        if (verts.empty())
            continue;
        unsigned int vao = 0, vbo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     verts.size() * sizeof(GCodeColoredVertex),
                     verts.data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GCodeColoredVertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GCodeColoredVertex), (void*)offsetof(GCodeColoredVertex, color));
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        vaos[i] = vao;
        vbos[i] = vbo;
    }
}
