#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
class Shader;

/**
 * @brief Renders a simple XYZ axis gizmo in screen space.
 */
class AxesRenderer {
public:
    AxesRenderer() = default;
    ~AxesRenderer();

    /** @brief Create buffers and load the shader. */
    void Init();
    /**
     * @brief Render the axes at an offset from the origin.
     * @param view View matrix.
     * @param proj Projection matrix.
     * @param offset Model space offset of the axes origin.
     */
    void Render(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &offset);

private:
    GLuint vao_ = 0, vbo_ = 0;
    std::unique_ptr<Shader> shader_;
};
