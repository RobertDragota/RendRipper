#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
class Shader;

class AxesRenderer {
public:
    AxesRenderer() = default;
    ~AxesRenderer();

    void Init();
    void Render(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &offset);

private:
    GLuint vao_ = 0, vbo_ = 0;
    std::unique_ptr<Shader> shader_;
};
