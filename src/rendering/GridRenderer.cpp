#include "GridRenderer.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>

GridRenderer::~GridRenderer()
{
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
}

void GridRenderer::Init(float halfX, float halfY)
{
    float hx = halfX;
    float hy = halfY;
    GLfloat vertices[] = { -hx, -hy, 0.f,  hx, -hy, 0.f,  hx, hy, 0.f,  -hx, hy, 0.f };
    GLuint indices[] = {0,1,2,2,3,0};
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(GLfloat),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    shader_ = std::make_unique<Shader>("../../resources/shaders/infinite_grid.vert",
                                      "../../resources/shaders/infinite_grid.frag");
}

void GridRenderer::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    if (!shader_ || vao_ == 0) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    shader_->use();
    shader_->setMat4("view", view);
    shader_->setMat4("projection", proj);
    shader_->setMat4("model", glm::mat4(1.0f));
    if (shader_->hasUniform("gridSpacing")) shader_->setFloat("gridSpacing", 5.0f);
    if (shader_->hasUniform("lineWidth")) shader_->setFloat("lineWidth", 0.5f);
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}
