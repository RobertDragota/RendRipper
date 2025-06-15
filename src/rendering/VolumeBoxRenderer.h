#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
class Shader;

/**
 * @brief Renders a wireframe box representing the printer volume.
 */
class VolumeBoxRenderer {
public:
    VolumeBoxRenderer() = default;
    ~VolumeBoxRenderer();

    /** Initialize buffers for the volume dimensions. */
    void Init(float halfX, float halfY, float height);
    void SetLineWidth(float width) { lineWidth_ = width; }
    /** Render the box with the provided matrices and color. */
    void Render(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &color);

private:
    GLuint vao_ = 0, vbo_ = 0;
    std::unique_ptr<Shader> shader_;
    float lineWidth_ = 1.0f;
};
