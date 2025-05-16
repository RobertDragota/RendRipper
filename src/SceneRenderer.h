// SceneRenderer.h
#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Model.h"
#include "Shader.h"
#include "GizmoController.h"
#include <glm/glm.hpp>
#include <memory>
#include "glad/glad.h"

class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    // Call once per frame before drawing any models
    void BeginScene(const glm::mat4& view);

    void RenderGrid();
    // Draw each model instance
    void RenderModel(const Model& model, Shader& shader, const Transform& t);
    // After all models are drawn, display the rendered texture
    void EndScene();

    [[nodiscard]] glm::mat4 GetViewMatrix()    const { return view_; }
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const { return proj_; }

    float getVolumeHalfX() const { return volumeHalfX_; }
    float getVolumeHalfY() const { return volumeHalfY_; }
    float getVolumeHalfZ() const { return volumeHalfZ_; }

private:
    void ResizeIfNeeded(int w, int h);

    glm::vec3 lightDirection_{  0.5f, -1.0f, 0.3f };

    unsigned int gridVAO_{0}, gridVBO_{0};
    std::unique_ptr<Shader> gridShader_;

    float volumeHalfX_ = 10.0f;
    float volumeHalfY_ = 10.0f;  // height
    float volumeHalfZ_ = 20.0f;

    GLuint boxVAO_, boxVBO_;

    unsigned int fbo_{0}, colorTex_{0}, rbo_{0}, whiteTex_{0};
    int fbWidth_{0}, fbHeight_{0};
    glm::mat4 view_{1.0f}, proj_{1.0f};

    void initVolumeBox();

    void RenderVolumeBox();
};

#endif // SCENERENDERER_H
