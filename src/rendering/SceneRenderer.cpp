#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

SceneRenderer::SceneRenderer(const std::string &printerDefJsonPath)
{
    if (!printerDefJsonPath.empty()) {
        std::ifstream in(printerDefJsonPath);
        if (in.is_open()) {
            try {
                json j; in >> j;
                auto o = j.at("overrides");
                float w = static_cast<float>(o.at("machine_width").at("value").get<double>());
                float d = static_cast<float>(o.at("machine_depth").at("value").get<double>());
                float h = static_cast<float>(o.at("machine_height").at("value").get<double>());
                volumeHalfX_ = w * 0.5f;
                volumeHalfY_ = d * 0.5f;
                volumeHeight_ = h;
                if (j.contains("metadata") && j["metadata"].contains("platform_offset")) {
                    auto arr = j["metadata"]["platform_offset"];
                    if (arr.is_array() && arr.size() == 3) {
                        float rx = static_cast<float>(arr[0].get<double>());
                        float ry = static_cast<float>(arr[1].get<double>());
                        float rz = static_cast<float>(arr[2].get<double>());
                        platformTransform_ = glm::rotate(glm::mat4(1.0f), glm::radians(rx), glm::vec3(1,0,0));
                        platformTransform_ = glm::rotate(platformTransform_, glm::radians(ry), glm::vec3(0,1,0));
                        platformTransform_ = glm::rotate(platformTransform_, glm::radians(rz), glm::vec3(0,0,1));
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "Warning: JSON parse error in SceneRenderer constructor: "
                          << e.what() << "\nFalling back to defaults.\n";
                volumeHalfX_ = 100.f; volumeHalfY_ = 100.f; volumeHeight_ = 200.f;
            }
        } else {
            std::cerr << "Warning: Could not open printerDefJsonPath: " << printerDefJsonPath << "\n";
            volumeHalfX_ = 100.f; volumeHalfY_ = 100.f; volumeHeight_ = 200.f;
        }
    } else {
        volumeHalfX_ = 100.f; volumeHalfY_ = 100.f; volumeHeight_ = 200.f;
    }

    InitializeDefaultTexture();
    framebuffer_.Init(viewportWidth_, viewportHeight_);
    InitializeGrid();
    InitializeVolumeBox();
    InitializeAxes();
    try {
        gcodeShader_ = std::make_unique<Shader>("../../resources/shaders/gcode_shader.vert",
                                               "../../resources/shaders/gcode_shader.frag");
    } catch (const std::exception &e) {
        std::cerr << "CRITICAL Error loading shaders in SceneRenderer: " << e.what() << std::endl;
        gcodeShader_.reset();
    }
    SetViewportSize(viewportWidth_, viewportHeight_);
}

SceneRenderer::~SceneRenderer()
{
    if (defaultWhiteTex_) glDeleteTextures(1, &defaultWhiteTex_);
}

void SceneRenderer::InitializeDefaultTexture()
{
    glGenTextures(1, &defaultWhiteTex_);
    glBindTexture(GL_TEXTURE_2D, defaultWhiteTex_);
    unsigned char whitePixel[4] = {255,255,255,255};
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D,0);
}


void SceneRenderer::InitializeGrid()
{
    gridRenderer_.Init(volumeHalfX_, volumeHalfY_);
}

void SceneRenderer::InitializeVolumeBox()
{
    volumeBoxRenderer_.Init(volumeHalfX_, volumeHalfY_, volumeHeight_);
}

void SceneRenderer::InitializeAxes()
{
    axesRenderer_.Init();
}

void SceneRenderer::SetViewportSize(int width, int height)
{
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    viewportWidth_ = width; viewportHeight_ = height;
    framebuffer_.Resize(width, height);
    projectionMatrix_ = glm::perspective(glm::radians(45.f), static_cast<float>(width)/static_cast<float>(height), 0.1f, 1000.f);
}


void SceneRenderer::BeginScene(const glm::mat4 &viewMatrix, const glm::vec3 &)
{
    viewMatrix_ = viewMatrix;
    framebuffer_.Bind();
    glViewport(0,0,viewportWidth_,viewportHeight_);
    glClearColor(0.10f,0.105f,0.11f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    RenderGridAndVolume();
}

void SceneRenderer::EndScene()
{
    framebuffer_.Unbind();
}

void SceneRenderer::RenderModel(const Model &model, Shader &shader, const Transform &transform)
{
    if (shader.ID == 0) return;
    shader.use();
    shader.setMat4("view", viewMatrix_);
    shader.setMat4("projection", projectionMatrix_);
    shader.setMat4("model", transform.getMatrix());
    glm::vec3 cameraWorldPosition = glm::vec3(glm::inverse(viewMatrix_)[3]);
    if (shader.hasUniform("lightDir")) shader.setVec3("lightDir", lightDirection_);
    if (shader.hasUniform("viewPos")) shader.setVec3("viewPos", cameraWorldPosition);
    if (shader.hasUniform("lightPos")) shader.setVec3("lightPos", cameraWorldPosition);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defaultWhiteTex_);
    if (shader.hasUniform("texture_diffuse1")) shader.setInt("texture_diffuse1",0);
    if (shader.hasUniform("objectColor")) shader.setVec4("objectColor", glm::vec4(0.7f,0.7f,0.7f,1.0f));
    model.Draw(shader);
}

void SceneRenderer::RenderGridAndVolume()
{
    gridRenderer_.Render(viewMatrix_, projectionMatrix_, platformTransform_);
    volumeBoxRenderer_.Render(viewMatrix_, projectionMatrix_, platformTransform_, gridColor_);
    RenderAxes();
}

void SceneRenderer::RenderAxes()
{
    axesRenderer_.Render(viewMatrix_, projectionMatrix_, platformTransform_, glm::vec3(-volumeHalfX_, -volumeHalfY_, 0.f));
}

void SceneRenderer::RenderGCodeLayer(int layerIndex)
{
    if (!gcodeModel_ || !gcodeShader_) return;
    gcodeShader_->use();
    glm::mat4 modelMat = platformTransform_;
    modelMat = glm::translate(modelMat, glm::vec3(-volumeHalfX_, -volumeHalfY_, 0.0f) + gcodeOffset_);
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);
    gcodeModel_->DrawLayer(layerIndex, *gcodeShader_);
}

void SceneRenderer::RenderGCodeUpToLayer(int maxLayerIndex)
{
    if (!gcodeModel_ || !gcodeShader_) return;
    gcodeShader_->use();
    glm::mat4 modelMat = platformTransform_;
    modelMat = glm::translate(modelMat, glm::vec3(-volumeHalfX_, -volumeHalfY_, 0.0f) + gcodeOffset_);
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);
    gcodeModel_->DrawUpToLayer(maxLayerIndex, *gcodeShader_);
}
