#include "SceneRenderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

SceneRenderer::SceneRenderer()
        : fbo_(0), colorTex_(0), rbo_(0), whiteTex_(0), fbWidth_(0), fbHeight_(0) {
    glGenTextures(1, &whiteTex_);
    glBindTexture(GL_TEXTURE_2D, whiteTex_);
    unsigned char white[3] = {255, 155, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &colorTex_);
    glGenRenderbuffers(1, &rbo_);
}

SceneRenderer::~SceneRenderer() {
    glDeleteFramebuffers(1, &fbo_);
    glDeleteTextures(1, &colorTex_);
    glDeleteRenderbuffers(1, &rbo_);
    glDeleteTextures(1, &whiteTex_);
}

void SceneRenderer::BeginImGuiScene() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Scene");
}

void SceneRenderer::Render(const Model &model,
                           Shader &shader,
                           const Transform &t,
                           glm::mat4 view) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int w = (int)avail.x, h = (int)avail.y;
    ResizeIfNeeded(w, h);

    float dist = model.radius * 2.5f;
    view_ = view;
    proj_ = glm::perspective(
            glm::radians(45.0f),
            float(w)/float(h),
            0.1f, dist*3.0f
    );
    modelMat_ =  glm::mat4(1.0f);
    modelMat_ = glm::translate(modelMat_, t.translation)
                * glm::toMat4(t.rotationQuat)
                * glm::scale(glm::mat4(1.0f), t.scale);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTex_);
    shader.setInt("texture_diffuse1", 0);
    shader.setMat4("view", view_);
    shader.setMat4("projection", proj_);
    shader.setMat4("model", modelMat_);
    model.Draw(shader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ImGui::Image((ImTextureID) (intptr_t) colorTex_, avail,
                 ImVec2{0, 1}, ImVec2{1, 0});
}

void SceneRenderer::EndImGuiScene() {
    ImGui::PopStyleVar();
    ImGui::End();
}

void SceneRenderer::ResizeIfNeeded(int w, int h) {
    if (w == fbWidth_ && h == fbHeight_) return;
    fbWidth_ = w;
    fbHeight_ = h;
    glBindTexture(GL_TEXTURE_2D, colorTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 SceneRenderer::GetViewMatrix() const { return view_; }

glm::mat4 SceneRenderer::GetProjectionMatrix() const { return proj_; }
