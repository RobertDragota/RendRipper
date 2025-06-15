#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
class Shader;

/**
 * @brief Utility that draws a flat ground grid.
 */
class GridRenderer {
public:
    GridRenderer() = default;
    ~GridRenderer();

    /** @brief Generate grid geometry of the given size. */
    void Init(float halfX, float halfY);
    /**
     * @brief Render the grid.
     * @param view View matrix.
     * @param proj Projection matrix.
     * @param lineColor Color of the grid lines.
     */
    void Render(const glm::mat4 &view,
                const glm::mat4 &proj,
                const glm::vec3 &lineColor);

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    std::unique_ptr<Shader> shader_;
};
