// SceneRenderer.h
#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Model.h"
#include "Shader.h"
#include "GizmoController.h"
#include <glm/glm.hpp>
#include "glad/glad.h"

class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    // Call once per frame before drawing any models
    void BeginScene(const glm::mat4& view);
    // Draw each model instance
    void RenderModel(const Model& model, Shader& shader, const Transform& t);
    // After all models are drawn, display the rendered texture
    void EndScene();

    glm::mat4 GetViewMatrix()    const { return view_; }
    glm::mat4 GetProjectionMatrix() const { return proj_; }

private:
    void ResizeIfNeeded(int w, int h);

    unsigned int fbo_{0}, colorTex_{0}, rbo_{0}, whiteTex_{0};
    int fbWidth_{0}, fbHeight_{0};
    glm::mat4 view_{1.0f}, proj_{1.0f};
};

#endif // SCENERENDERER_H
