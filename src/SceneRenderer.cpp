// SceneRenderer.cpp
#include "SceneRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>


SceneRenderer::SceneRenderer() {
    // 1×1 white texture (fallback)
    glGenTextures(1, &whiteTex_);
    glBindTexture(GL_TEXTURE_2D, whiteTex_);
    unsigned char whiteColor[3] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whiteColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // FBO + color texture + depth/stencil RBO
    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &colorTex_);
    glGenRenderbuffers(1, &rbo_);


    gridShader_ = std::make_unique<Shader>(
            "../resources/shaders/plate_shader.vert",
            "../resources/shaders/plate_shader.frag"
    );
    // build X–Z grid lines from x,z = –10…10
    std::vector<glm::vec3> lines;
    for (int i = -10; i <= 10; ++i) {
        lines.emplace_back(i, 0, -10);
        lines.emplace_back(i, 0, 10);
        lines.emplace_back(-10, 0, i);
        lines.emplace_back(10, 0, i);
    }
    glGenVertexArrays(1, &gridVAO_);
    glGenBuffers(1, &gridVBO_);
    glBindVertexArray(gridVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO_);
    glBufferData(GL_ARRAY_BUFFER,
                 lines.size() * sizeof(glm::vec3),
                 lines.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3), nullptr);
    glBindVertexArray(0);


}

SceneRenderer::~SceneRenderer() {
    glDeleteFramebuffers(1, &fbo_);
    glDeleteTextures(1, &colorTex_);
    glDeleteRenderbuffers(1, &rbo_);
    glDeleteTextures(1, &whiteTex_);
    glDeleteVertexArrays(1, &gridVAO_);
    glDeleteBuffers(1, &gridVBO_);
}

void SceneRenderer::BeginScene(const glm::mat4 &view) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Scene");

    ImVec2 avail = ImGui::GetContentRegionAvail();
    int w = (int) avail.x, h = (int) avail.y;
    ResizeIfNeeded(w, h);

    view_ = view;
    proj_ = glm::perspective(glm::radians(45.0f),
                             float(w) / float(h),
                             0.1f, 100.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderGrid();
}

void SceneRenderer::RenderModel(const Model &model,
                                Shader &shader,
                                const Transform &t) {
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTex_);
    shader.setInt("texture_diffuse1", 0);
    shader.setMat4("view", view_);
    shader.setMat4("projection", proj_);

    glm::mat4 modelMat =
            glm::translate(glm::mat4(1.0f), t.translation)
            * glm::toMat4(t.rotationQuat)
            * glm::scale(glm::mat4(1.0f), t.scale);
    shader.setMat4("model", modelMat);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID) (intptr_t) colorTex_, avail,
                 ImVec2{0, 1}, ImVec2{1, 0});
    shader.setVec3("lightDir", lightDirection_);
    shader.setVec4("objectColor", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

    model.Draw(shader);
}

void SceneRenderer::EndScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ImGui::PopStyleVar();
    ImGui::End();
}

void SceneRenderer::ResizeIfNeeded(int w, int h) {
    if (w == fbWidth_ && h == fbHeight_) return;
    fbWidth_ = w;
    fbHeight_ = h;

    glBindTexture(GL_TEXTURE_2D, colorTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, colorTex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::RenderGrid() {
    gridShader_->use();
    gridShader_->setMat4("view", view_);
    gridShader_->setMat4("projection", proj_);
    gridShader_->setMat4("model", glm::mat4(1.0f));
    glBindVertexArray(gridVAO_);
    glDrawArrays(GL_LINES, 0, (10 - (-10) + 1) * 4);
    glBindVertexArray(0);
}
