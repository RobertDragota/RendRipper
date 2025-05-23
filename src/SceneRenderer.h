#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Shader;
class Model;
struct Transform;

class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    void BeginScene(const glm::mat4& viewMatrix, const glm::vec3& cameraWorldPosition);
    void EndScene();
    void RenderModel(const Model& model, Shader& shader, const Transform& transform);
    void SetViewportSize(int width, int height);

    GLuint GetSceneTexture() const { return colorTex_; }
    const glm::mat4& GetViewMatrix() const { return viewMatrix_; }
    const glm::mat4& GetProjectionMatrix() const { return projectionMatrix_; }
    void SetGridColor(const glm::vec3& color) { gridColor_ = color; }

private:
    void InitializeDefaultTexture();
    void InitializeFBO();
    void InitializeGrid(); // For shader-based infinite grid quad
    void InitializeVolumeBox(); // For line-based volume box
    void ResizeFBOIfNeeded(int width, int height);
    void RenderGridAndVolume();

    GLuint fbo_ = 0;
    GLuint colorTex_ = 0;
    GLuint rboDepthStencil_ = 0;
    GLuint defaultWhiteTex_ = 0;

    // For shader-based infinite grid
    GLuint gridVAO_ = 0, gridVBO_ = 0, gridEBO_ = 0;
    std::unique_ptr<Shader> gridShader_; // Uses infinite_grid.vert/frag

    // For line-based volume box
    GLuint volumeBoxVAO_ = 0, volumeBoxVBO_ = 0;
    std::unique_ptr<Shader> volumeBoxShader_; // Uses simple_line.vert/frag

    glm::vec3 gridColor_ = glm::vec3(0.4f, 0.4f, 0.45f); // Color for volume box lines
    // Infinite grid shader has its own colors

    int viewportWidth_ = 1;
    int viewportHeight_ = 1;

    glm::mat4 viewMatrix_;
    glm::mat4 projectionMatrix_;

    glm::vec3 lightDirection_ = glm::normalize(glm::vec3(0.5f, 0.5f, 1.0f));

    float volumeHalfX_ = 10.0f;
    float volumeHalfY_ = 10.0f;
    float volumeHeight_ = 15.0f;
};