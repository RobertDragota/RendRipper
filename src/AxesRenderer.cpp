#include "AxesRenderer.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>

AxesRenderer::~AxesRenderer()
{
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
}

void AxesRenderer::Init()
{
    const float axisLength = 20.0f;
    GLfloat axisVertices[] = {
        0.f,0.f,0.f,  1.f,0.f,0.f,  axisLength,0.f,0.f, 1.f,0.f,0.f,
        0.f,0.f,0.f,  0.f,1.f,0.f,  0.f,axisLength,0.f, 0.f,1.f,0.f,
        0.f,0.f,0.f,  0.f,0.f,1.f,  0.f,0.f,axisLength, 0.f,0.f,1.f
    };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(GLfloat),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(GLfloat),(void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    shader_ = std::make_unique<Shader>("../../resources/shaders/simple_colored_line.vert",
                                       "../../resources/shaders/simple_colored_line.frag");
}

void AxesRenderer::Render(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &offset)
{
    if (!shader_ || vao_ == 0) return;
    GLboolean depthTestEnabled;
    GLboolean depthWriteMask;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteMask);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    shader_->use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), offset);
    shader_->setMat4("model", model);
    shader_->setMat4("view", view);
    shader_->setMat4("projection", proj);
    glBindVertexArray(vao_);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glDepthMask(depthWriteMask);
}
