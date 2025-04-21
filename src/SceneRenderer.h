#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include "Model.h"
#include "Shader.h"
#include "GizmoController.h"  // for Transform

class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    void BeginImGuiScene();                         // ImGui::Begin + style
    void Render(const Model& m, Shader& s, const Transform& t, glm::mat4 view);
    void EndImGuiScene();                           // ImGui::End + pop

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

private:
    void ResizeIfNeeded(int w, int h);

    unsigned int fbo_, colorTex_, rbo_, whiteTex_;
    glm::mat4 view_, proj_, modelMat_;
    int fbWidth_, fbHeight_;
};
