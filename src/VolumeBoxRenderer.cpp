#include "VolumeBoxRenderer.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

VolumeBoxRenderer::~VolumeBoxRenderer()
{
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
}

void VolumeBoxRenderer::Init(float halfX, float halfY, float height)
{
    float hx = halfX; float hy = halfY; float hz = height;
    glm::vec3 B0(-hx, -hy, 0), B1(hx, -hy, 0), B2(hx, hy, 0), B3(-hx, hy, 0);
    glm::vec3 T0(-hx, -hy, hz), T1(hx, -hy, hz), T2(hx, hy, hz), T3(-hx, hy, hz);
    std::vector<glm::vec3> lines = { B0,B1,B1,B2,B2,B3,B3,B0, T0,T1,T1,T2,T2,T3,T3,T0, B0,T0,B1,T1,B2,T2,B3,T3 };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, lines.size()*sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(glm::vec3),(void*)0);
    glBindVertexArray(0);
    shader_ = std::make_unique<Shader>("../../resources/shaders/simple_line.vert",
                                       "../../resources/shaders/simple_line.frag");
}

void VolumeBoxRenderer::Render(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &color)
{
    if (!shader_ || vao_ == 0) return;
    shader_->use();
    shader_->setMat4("view", view);
    shader_->setMat4("projection", proj);
    shader_->setMat4("model", glm::mat4(1.0f));
    if (shader_->hasUniform("lineColor")) shader_->setVec3("lineColor", color);
    glBindVertexArray(vao_);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}
