#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <vector>


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
    framebuffer_.Init(viewportWidth_, viewportHeight_, 4);
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
    try {
        deferredGeomShader_ = std::make_unique<Shader>("../../resources/shaders/deferred_geom.vert",
                                                       "../../resources/shaders/deferred_geom.frag");
        deferredLightingShader_ = std::make_unique<Shader>("../../resources/shaders/deferred_light.vert",
                                                           "../../resources/shaders/deferred_light.frag");
    } catch (const std::exception &e) {
        std::cerr << "CRITICAL Error loading deferred shaders: " << e.what() << std::endl;
        deferredGeomShader_.reset();
        deferredLightingShader_.reset();
    }
    InitGBuffer();
    InitQuad();
    SetViewportSize(viewportWidth_, viewportHeight_);
}

SceneRenderer::~SceneRenderer()
{
    if (gPosition_) glDeleteTextures(1, &gPosition_);
    if (gNormal_) glDeleteTextures(1, &gNormal_);
    if (gAlbedo_) glDeleteTextures(1, &gAlbedo_);
    if (gDepthRBO_) glDeleteRenderbuffers(1, &gDepthRBO_);
    if (gBuffer_) glDeleteFramebuffers(1, &gBuffer_);
    if (quadVAO_) glDeleteVertexArrays(1, &quadVAO_);
    if (quadVBO_) glDeleteBuffers(1, &quadVBO_);
}

void SceneRenderer::InitializeDefaultTexture()
{
    defaultWhiteTex_ = TextureCache::GetSolidColorTexture(255, 255, 255, 255);
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
    ResizeGBuffer(width, height);
    projectionMatrix_ = glm::perspective(glm::radians(45.f), static_cast<float>(width)/static_cast<float>(height), 0.1f, 1000.f);
}


void SceneRenderer::BeginScene(const glm::mat4 &viewMatrix, const glm::vec3 &)
{
    viewMatrix_ = viewMatrix;
    inGeometryPass_ = true;
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_);
    glViewport(0,0,viewportWidth_,viewportHeight_);
    glClearColor(0.10f,0.105f,0.11f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    RenderGridAndVolume();
}

void SceneRenderer::EndScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    inGeometryPass_ = false;

    framebuffer_.Bind();
    glDisable(GL_DEPTH_TEST);
    if (deferredLightingShader_ && quadVAO_) {
        deferredLightingShader_->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        deferredLightingShader_->setInt("gPosition",0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        deferredLightingShader_->setInt("gNormal",1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo_);
        deferredLightingShader_->setInt("gAlbedo",2);
        glm::vec3 camPos = glm::vec3(glm::inverse(viewMatrix_)[3]);
        if (deferredLightingShader_->hasUniform("lightDir"))
            deferredLightingShader_->setVec3("lightDir", lightDirection_);
        if (deferredLightingShader_->hasUniform("lightPos"))
            deferredLightingShader_->setVec3("lightPos", camPos);
        if (deferredLightingShader_->hasUniform("viewPos"))
            deferredLightingShader_->setVec3("viewPos", camPos);
        RenderQuad();
    }
    framebuffer_.Unbind();
    glEnable(GL_DEPTH_TEST);
}

void SceneRenderer::RenderModel(const Model &model, Shader &shader, const Transform &transform)
{
    Shader* useShader = inGeometryPass_ ? deferredGeomShader_.get() : &shader;
    if (!useShader || useShader->ID == 0) return;
    useShader->use();
    useShader->setMat4("view", viewMatrix_);
    useShader->setMat4("projection", projectionMatrix_);
    useShader->setMat4("model", transform.getMatrix());
    glm::vec3 cameraWorldPosition = glm::vec3(glm::inverse(viewMatrix_)[3]);
    if (!inGeometryPass_) {
        if (useShader->hasUniform("lightDir")) useShader->setVec3("lightDir", lightDirection_);
        if (useShader->hasUniform("viewPos")) useShader->setVec3("viewPos", cameraWorldPosition);
        if (useShader->hasUniform("lightPos")) useShader->setVec3("lightPos", cameraWorldPosition);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defaultWhiteTex_ ? defaultWhiteTex_->id : 0);
    if (useShader->hasUniform("texture_diffuse1")) useShader->setInt("texture_diffuse1",0);
    if (useShader->hasUniform("objectColor")) useShader->setVec4("objectColor", glm::vec4(0.7f,0.7f,0.7f,1.0f));
    model.Draw(*useShader);
}

void SceneRenderer::RenderGridAndVolume()
{
    gridRenderer_.Render(viewMatrix_, projectionMatrix_, gridColor_);
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
    glm::mat4 modelMat(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(-volumeHalfX_, -volumeHalfY_, 0.0f) + gcodeOffset_);
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);
    gcodeModel_->DrawUpToLayer(maxLayerIndex, *gcodeShader_);
}

void SceneRenderer::InitGBuffer()
{
    glGenFramebuffers(1, &gBuffer_);
    glGenTextures(1, &gPosition_);
    glGenTextures(1, &gNormal_);
    glGenTextures(1, &gAlbedo_);
    glGenRenderbuffers(1, &gDepthRBO_);
    ResizeGBuffer(viewportWidth_, viewportHeight_);
}

void SceneRenderer::ResizeGBuffer(int width, int height)
{
    if (!gBuffer_) return;
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_);

    glBindTexture(GL_TEXTURE_2D, gPosition_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition_, 0);

    glBindTexture(GL_TEXTURE_2D, gNormal_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal_, 0);

    glBindTexture(GL_TEXTURE_2D, gAlbedo_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo_, 0);

    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRBO_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthRBO_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "GBuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::InitQuad()
{
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };
    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);
    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void SceneRenderer::RenderQuad()
{
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
