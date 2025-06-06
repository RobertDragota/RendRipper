#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include "GCodeModel.h"
#include "GridRenderer.h"
#include "VolumeBoxRenderer.h"
#include "AxesRenderer.h"
using json = nlohmann::json;

class Shader;
class Model;
struct Transform;

class SceneRenderer {
public:
    explicit SceneRenderer(const std::string &printerDefJsonPath = "");
    ~SceneRenderer();

    void BeginScene(const glm::mat4 &viewMatrix, const glm::vec3 &cameraWorldPosition);
    void EndScene();
    void RenderModel(const Model &model, Shader &shader, const Transform &transform);
    void RenderGCodeLayer(int layerIndex);
    void RenderGCodeUpToLayer(int maxLayerIndex);
    void SetViewportSize(int width, int height);

    GLuint GetSceneTexture() const { return colorTex_; }
    const glm::mat4 &GetViewMatrix() const { return viewMatrix_; }
    const glm::mat4 &GetProjectionMatrix() const { return projectionMatrix_; }
    void SetGridColor(const glm::vec3 &color) { gridColor_ = color; }

    float GetBedHalfWidth() const { return volumeHalfX_; }
    float GetBedHalfDepth() const { return volumeHalfY_; }
    float GetPrintHeight() const { return volumeHeight_; }

    void SetGCodeModel(std::shared_ptr<GCodeModel> gcodeModel) { gcodeModel_ = gcodeModel; }
    int currentGCodeLayerIndex_ = -1;

private:
    void InitializeDefaultTexture();
    void InitializeFBO();
    void InitializeGrid();
    void InitializeVolumeBox();
    void InitializeAxes();
    void ResizeFBOIfNeeded(int width, int height);
    void RenderGridAndVolume();
    void RenderAxes();

    GLuint fbo_ = 0;
    GLuint colorTex_ = 0;
    GLuint rboDepthStencil_ = 0;
    GLuint defaultWhiteTex_ = 0;

    glm::vec3 gridColor_ = glm::vec3(0.4f, 0.4f, 0.45f);
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

    GridRenderer     gridRenderer_;
    VolumeBoxRenderer volumeBoxRenderer_;
    AxesRenderer     axesRenderer_;
};
