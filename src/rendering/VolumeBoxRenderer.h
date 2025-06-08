#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
class Shader;

class VolumeBoxRenderer {
public:
    VolumeBoxRenderer() = default;
    ~VolumeBoxRenderer();

    void Init(float halfX, float halfY, float height);
    void Render(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &color, const glm::vec3 &offset = glm::vec3(0.0f));

private:
    GLuint vao_ = 0, vbo_ = 0;
    std::unique_ptr<Shader> shader_;
};
