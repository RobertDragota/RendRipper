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
    InitializeFBO();
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
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    if (colorTex_) glDeleteTextures(1, &colorTex_);
    if (rboDepthStencil_) glDeleteRenderbuffers(1, &rboDepthStencil_);
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

void SceneRenderer::InitializeFBO()
{
    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &colorTex_);
    glGenRenderbuffers(1, &rboDepthStencil_);
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
    ResizeFBOIfNeeded(width, height);
    projectionMatrix_ = glm::perspective(glm::radians(45.f), static_cast<float>(width)/static_cast<float>(height), 0.1f, 1000.f);
}

void SceneRenderer::ResizeFBOIfNeeded(int width, int height)
{
    if (!fbo_ || !colorTex_ || !rboDepthStencil_) {
        InitializeFBO();
        if (!fbo_ || !colorTex_ || !rboDepthStencil_) {
            std::cerr << "E:FBO Comp Fail\n"; return; }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glBindTexture(GL_TEXTURE_2D, colorTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex_, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil_);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "E:FBO Not Comp!" << std::endl;
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void SceneRenderer::BeginScene(const glm::mat4 &viewMatrix, const glm::vec3 &)
{
    if (!fbo_) {
        std::cerr << "E:FBO Not Init BS\n"; return; }
    viewMatrix_ = viewMatrix;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0,0,viewportWidth_,viewportHeight_);
    glClearColor(0.10f,0.105f,0.11f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    RenderGridAndVolume();
}

void SceneRenderer::EndScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    gridRenderer_.Render(viewMatrix_, projectionMatrix_);
    volumeBoxRenderer_.Render(viewMatrix_, projectionMatrix_, gridColor_);
    RenderAxes();
}

void SceneRenderer::RenderAxes()
{
    axesRenderer_.Render(viewMatrix_, projectionMatrix_, glm::vec3(-volumeHalfX_, -volumeHalfY_, 0.f));
}

void SceneRenderer::RenderGCodeLayer(int layerIndex)
{
    if (!gcodeModel_ || !gcodeShader_) return;
    gcodeShader_->use();
    glm::mat4 modelMat(1.0f);
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);
    gcodeModel_->DrawLayer(layerIndex, *gcodeShader_);
}

void SceneRenderer::RenderGCodeUpToLayer(int maxLayerIndex)
{
    if (!gcodeModel_ || !gcodeShader_) return;
    gcodeShader_->use();
    glm::mat4 modelMat(1.0f);
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);
    gcodeModel_->DrawUpToLayer(maxLayerIndex, *gcodeShader_);
}
