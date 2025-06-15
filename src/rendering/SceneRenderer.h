#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include "GCodeModel.h"
#include "GridRenderer.h"
#include "VolumeBoxRenderer.h"
#include "AxesRenderer.h"
#include "FrameBuffer.h"
#include "TextureCache.h"
using json = nlohmann::json;

class Shader;
class Model;
struct Transform;

/**
 * @brief High level renderer handling models, grid and helpers.
 */
class SceneRenderer {
public:
    explicit SceneRenderer(const std::string &printerDefJsonPath );
    ~SceneRenderer();

    /** Prepare the frame for drawing. */
    void BeginScene(const glm::mat4 &viewMatrix, const glm::vec3 &cameraWorldPosition);
    /** Finish rendering. */
    void EndScene();
    /** Render a regular 3D model. */
    void RenderModel(const Model &model, Shader &shader, const Transform &transform);
    /** Render a single G-code layer. */
    void RenderGCodeLayer(int layerIndex);
    /** Render all layers up to the specified one. */
    void RenderGCodeUpToLayer(int maxLayerIndex);
    /** Resize the internal framebuffer. */
    void SetViewportSize(int width, int height);

    GLuint GetSceneTexture() const { return framebuffer_.GetColorTexture(); }
    const glm::mat4 &GetViewMatrix() const { return viewMatrix_; }
    const glm::mat4 &GetProjectionMatrix() const { return projectionMatrix_; }
    void SetGridColor(const glm::vec3 &color) { gridColor_ = color; }

    float GetBedHalfWidth() const { return volumeHalfX_; }
    float GetBedHalfDepth() const { return volumeHalfY_; }
    float GetPrintHeight() const { return volumeHeight_; }

    void SetGCodeModel(std::shared_ptr<GCodeModel> gcodeModel) { gcodeModel_ = gcodeModel; }
    void SetGCodeOffset(const glm::vec3& offset) { gcodeOffset_ = offset; }
    int currentGCodeLayerIndex_ = -1;

private:
    /** Load a fallback white texture. */
    void InitializeDefaultTexture();
    /** Create grid renderer resources. */
    void InitializeGrid();
    /** Setup the printer volume wireframe. */
    void InitializeVolumeBox();
    /** Setup the small axis widget. */
    void InitializeAxes();
    /** Render grid and volume helpers. */
    void RenderGridAndVolume();
    /** Draw the axis widget. */
    void RenderAxes();

    FrameBuffer framebuffer_;
    std::shared_ptr<Texture> defaultWhiteTex_;

    glm::vec3 gridColor_ = glm::vec3(0.6f, 0.6f, 0.65f);
    int viewportWidth_ = 1;
    int viewportHeight_ = 1;

    glm::mat4 viewMatrix_;
    glm::mat4 projectionMatrix_;
    glm::vec3 lightDirection_ = glm::normalize(glm::vec3(0.5f, 0.5f, 1.0f));

    float volumeHalfX_;
    float volumeHalfY_;
    float volumeHeight_;

    std::shared_ptr<GCodeModel> gcodeModel_;
    std::unique_ptr<Shader> gcodeShader_;

    glm::vec3 gcodeOffset_ = glm::vec3(0.0f);

    GridRenderer     gridRenderer_;
    VolumeBoxRenderer volumeBoxRenderer_;
    AxesRenderer     axesRenderer_;
};
