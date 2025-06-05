#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

SceneRenderer::SceneRenderer(const std::string &printerDefJsonPath)
{
    // 1. Attempt to read printer‐definition JSON and extract bed dimensions.
    //    We expect, under "overrides", keys "machine_width", "machine_depth", "machine_height",
    //    each with a sub‐field "value". If anything fails, fall back to safe defaults.
    if (!printerDefJsonPath.empty())
        {
        std::ifstream in(printerDefJsonPath);
        if (in.is_open())
            {
            try
                {
                json j;
                in >> j; //
                auto overrides = j.at("overrides");
                float machineW = static_cast<float>(overrides.at("machine_width").at("value").get<double>());
                float machineD = static_cast<float>(overrides.at("machine_depth").at("value").get<double>());
                float machineH = static_cast<float>(overrides.at("machine_height").at("value").get<double>());
                volumeHalfX_ = machineW * 0.5f;
                volumeHalfY_ = machineD * 0.5f;
                volumeHeight_ = machineH;
                }
            catch (const std::exception &e)
                {
                std::cerr << "Warning: JSON parse error in SceneRenderer constructor: "
                        << e.what() << "\nFalling back to defaults.\n";
                // Defaults (e.g., 200×200×200 mm bed)
                volumeHalfX_ = 100.0f;
                volumeHalfY_ = 100.0f;
                volumeHeight_ = 200.0f;
                }
            }
        else
            {
            std::cerr << "Warning: Could not open printerDefJsonPath: " << printerDefJsonPath << "\n";
            volumeHalfX_ = 100.0f;
            volumeHalfY_ = 100.0f;
            volumeHeight_ = 200.0f;
            }
        }
    else
        {
        // No JSON path provided; use reasonable defaults:
        volumeHalfX_ = 100.0f;
        volumeHalfY_ = 100.0f;
        volumeHeight_ = 200.0f;
        }

    InitializeDefaultTexture();
    InitializeFBO();
    InitializeGrid(); // Now builds a finite quad covering the bed extents (see below)
    InitializeVolumeBox(); // Builds a wireframe box using volumeHalfX_/Y_/Height_
    InitializeAxes();
    try
        {
        gridShader_ = std::make_unique<Shader>(
            "../../resources/shaders/infinite_grid.vert",
            "../../resources/shaders/infinite_grid.frag"
        );
        if (!gridShader_ || gridShader_->ID == 0)
            throw std::runtime_error("Infinite Grid shader failed to compile/link.");

        volumeBoxShader_ = std::make_unique<Shader>(
            "../../resources/shaders/simple_line.vert",
            "../../resources/shaders/simple_line.frag"
        );
        if (!volumeBoxShader_ || volumeBoxShader_->ID == 0)
            throw std::runtime_error("Simple Line (Volume Box) shader failed to compile/link.");

        axesShader_ = std::make_unique<Shader>("../../resources/shaders/simple_colored_line.vert",
                                               "../../resources/shaders/simple_colored_line.frag");
        if (!axesShader_ || axesShader_->ID == 0)
            throw std::runtime_error("Axes shader failed to compile/link.");

        gcodeShader_ = std::make_unique<Shader>(
            "../../resources/shaders/gcode_shader.vert",
            "../../resources/shaders/gcode_shader.frag"
        );
        if (!gcodeShader_ || gcodeShader_->ID == 0)
            throw std::runtime_error("G-code shader failed to compile/link.");
        }
    catch (const std::exception &e)
        {
        std::cerr << "CRITICAL Error loading shaders in SceneRenderer: " << e.what() << std::endl;
        if (gridShader_ && gridShader_->ID == 0)
            gridShader_ = nullptr;
        if (volumeBoxShader_ && volumeBoxShader_->ID == 0)
            volumeBoxShader_ = nullptr;
        if (axesShader_ && axesShader_->ID == 0)
            axesShader_ = nullptr;
        }
    SetViewportSize(viewportWidth_, viewportHeight_);
}

SceneRenderer::~SceneRenderer()
{
    if (fbo_)
        glDeleteFramebuffers(1, &fbo_);
    if (colorTex_)
        glDeleteTextures(1, &colorTex_);
    if (rboDepthStencil_)
        glDeleteRenderbuffers(1, &rboDepthStencil_);
    if (defaultWhiteTex_)
        glDeleteTextures(1, &defaultWhiteTex_);
    if (gridVAO_)
        glDeleteVertexArrays(1, &gridVAO_);
    if (gridVBO_)
        glDeleteBuffers(1, &gridVBO_);
    if (gridEBO_)
        glDeleteBuffers(1, &gridEBO_);
    if (volumeBoxVAO_)
        glDeleteVertexArrays(1, &volumeBoxVAO_);
    if (volumeBoxVBO_)
        glDeleteBuffers(1, &volumeBoxVBO_);
}

void SceneRenderer::InitializeDefaultTexture()
{

    glGenTextures(1, &defaultWhiteTex_);
    glBindTexture(GL_TEXTURE_2D, defaultWhiteTex_);
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SceneRenderer::InitializeFBO()
{

    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &colorTex_);
    glGenRenderbuffers(1, &rboDepthStencil_);
}

void SceneRenderer::InitializeGrid()
{
    // For shader-based infinite grid (large quad)
    float hx = volumeHalfX_;
    float hy = volumeHalfY_;
    GLfloat vertices[] = {
            -hx, -hy, 0.0f, // bottom-left
            hx, -hy, 0.0f, // bottom-right
            hx, hy, 0.0f, // top-right
            -hx, hy, 0.0f // top-left
            };
    GLuint indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &gridVAO_);
    glGenBuffers(1, &gridVBO_);
    glGenBuffers(1, &gridEBO_);

    glBindVertexArray(gridVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind VAO first, then other buffers if needed
}

void SceneRenderer::InitializeVolumeBox()
{
    /* ... as before ... */
    float hx = volumeHalfX_;
    float hy = volumeHalfY_;
    float hz = volumeHeight_;
    glm::vec3 B0(-hx, -hy, 0), B1(hx, -hy, 0), B2(hx, hy, 0), B3(-hx, hy, 0);
    glm::vec3 T0(-hx, -hy, hz), T1(hx, -hy, hz), T2(hx, hy, hz), T3(-hx, hy, hz);
    std::vector<glm::vec3> lines = {
            B0, B1, B1, B2, B2, B3, B3, B0, T0, T1, T1, T2, T2, T3, T3, T0, B0, T0, B1, T1, B2,
            T2, B3, T3
            };
    if (lines.empty())
        return;
    glGenVertexArrays(1, &volumeBoxVAO_);
    glGenBuffers(1, &volumeBoxVBO_);
    glBindVertexArray(volumeBoxVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, volumeBoxVBO_);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glBindVertexArray(0);
}

void SceneRenderer::InitializeAxes()
{

        // ===== Build a small VBO/VAO for three world‐space colored axes =====
        const float axisLength = 20.0f; // each axis = 20 mm long for visibility
        // interleaved: (pos.x, pos.y, pos.z, color.r, color.g, color.b)
        GLfloat axisVertices[] = {
                // X axis → red
                 0.f,  0.f,  0.f,   1.f, 0.f, 0.f,
                 axisLength, 0.f,  0.f,   1.f, 0.f, 0.f,
                // Y axis → green
                 0.f,  0.f,  0.f,   0.f, 1.f, 0.f,
                 0.f,  axisLength, 0.f,   0.f, 1.f, 0.f,
                // Z axis → blue
                 0.f,  0.f,  0.f,   0.f, 0.f, 1.f,
                 0.f,  0.f,  axisLength,   0.f, 0.f, 1.f
            };

        glGenVertexArrays(1, &axesVAO_);
        glGenBuffers(1, &axesVBO_);
        glBindVertexArray(axesVAO_);

        glBindBuffer(GL_ARRAY_BUFFER, axesVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

        // Position attribute at location = 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        // Color attribute at location = 1
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void SceneRenderer::SetViewportSize(int width, int height)
{
    if (width <= 0)
        width = 1;
    if (height <= 0)
        height = 1;
    viewportWidth_ = width;
    viewportHeight_ = height;
    ResizeFBOIfNeeded(width, height);
    projectionMatrix_ = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height),
                                         0.1f, 1000.0f);
}

void SceneRenderer::ResizeFBOIfNeeded(int width, int height)
{

    if (!fbo_ || !colorTex_ || !rboDepthStencil_)
        {
        InitializeFBO();
        if (!fbo_ || !colorTex_ || !rboDepthStencil_)
            {
            std::cerr << "E:FBO Comp Fail\n";
            return;
            }
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
        std::cerr << "E:FBO Not Comp! S:0x" << std::hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::dec
                << std::endl;
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
SceneRenderer::BeginScene(const glm::mat4 &viewMatrix, const glm::vec3 & /*cameraWorldPos*/)
{

    if (!fbo_)
        {
        std::cerr << "E:FBO Not Init BS\n";
        return;
        }
    viewMatrix_ = viewMatrix;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, viewportWidth_, viewportHeight_);
    glClearColor(0.10f, 0.105f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    RenderGridAndVolume();

}

void SceneRenderer::EndScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
SceneRenderer::RenderModel(const Model &model, Shader &shader, const Transform &transform)
{
    /* ... as before ... */
    if (shader.ID == 0)
        return;
    shader.use();
    shader.setMat4("view", viewMatrix_);
    shader.setMat4("projection", projectionMatrix_);
    shader.setMat4("model", transform.getMatrix());
    glm::vec3 cameraWorldPosition = glm::vec3(glm::inverse(viewMatrix_)[3]);

    if (shader.hasUniform("lightDir"))
        {
        shader.setVec3("lightDir", lightDirection_);
        }
    if (shader.hasUniform("viewPos"))
        {
        shader.setVec3("viewPos", cameraWorldPosition);
        }
    if (shader.hasUniform("lightPos"))
        {
        shader.setVec3("lightPos", cameraWorldPosition);
        }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defaultWhiteTex_);
    if (shader.hasUniform("texture_diffuse1"))
        shader.setInt("texture_diffuse1", 0);
    if (shader.hasUniform("objectColor"))
        shader.setVec4("objectColor", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
    model.Draw(shader);
}

void SceneRenderer::RenderGridAndVolume()
{
    // Render Infinite Grid (using infinite_grid shaders)
    if (gridShader_ && gridShader_->ID != 0 && gridVAO_ != 0 && gridEBO_ != 0)
        {
        glEnable(GL_BLEND); // Enable blending for potentially semi-transparent grid lines
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Render grid without writing to depth buffer so models don't z-fight

        gridShader_->use();
        gridShader_->setMat4("view", viewMatrix_);
        gridShader_->setMat4("projection", projectionMatrix_);
        gridShader_->setMat4("model", glm::mat4(1.0f)); // Grid quad is already at Z=0
        if (gridShader_->hasUniform("gridSpacing"))
            gridShader_->setFloat("gridSpacing", 5.0f);
        if (gridShader_->hasUniform("lineWidth"))
            gridShader_->setFloat("lineWidth", 0.5f); // Adjust as needed
        // Colors are hardcoded in your infinite_grid.frag

        glBindVertexArray(gridVAO_);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE); // Re-enable depth writing
        //glDisable(GL_BLEND);
        }

    // Render Volume Box
    if (volumeBoxShader_ && volumeBoxShader_->ID != 0 && volumeBoxVAO_ != 0)
        {
        volumeBoxShader_->use();
        volumeBoxShader_->setMat4("view", viewMatrix_);
        volumeBoxShader_->setMat4("projection", projectionMatrix_);
        volumeBoxShader_->setMat4("model", glm::mat4(1.0f));
        if (volumeBoxShader_->hasUniform("lineColor"))
            {
            volumeBoxShader_->setVec3("lineColor", gridColor_);
            }
        glBindVertexArray(volumeBoxVAO_);
        glDrawArrays(GL_LINES, 0, 24);
        glBindVertexArray(0);
        }
    RenderAxes();

    RenderGCodeUpToLayer(currentGCodeLayerIndex_);
}

void SceneRenderer::RenderAxes()
{
    if (!axesShader_ || axesVAO_ == 0)
        return;

    // 0) Save current GL depth state
    GLboolean depthTestEnabled;
    GLboolean depthWriteMask;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);     // was glEnable(GL_DEPTH_TEST) ?
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteMask);  // was glDepthMask(GL_TRUE) ?

    // 1) Disable depth testing & depth writes, so the axes always appear on top
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // 2) Use our colored-line shader
    axesShader_->use();

    // 3) Set uniforms: model → translate to (-halfX, -halfY, 0), plus view/projection
    glm::mat4 model = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(-volumeHalfX_, -volumeHalfY_, 0.0f)
    );
    axesShader_->setMat4("model", model);
    axesShader_->setMat4("view",  viewMatrix_);
    axesShader_->setMat4("projection", projectionMatrix_);

    // 4) Bind VAO and draw 3 lines (6 vertices total)
    glBindVertexArray(axesVAO_);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);

    // 5) Restore previous depth state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(depthWriteMask);
}


// ─── RenderGCodeLayer ────────────────────────────────────────────────────────────
// Draw only one layer from the GCodeModel (if present)
// We assume `viewMatrix_` and `projectionMatrix_` have been set by BeginScene().
void SceneRenderer::RenderGCodeLayer(int layerIndex) {
    if (!gcodeModel_ || !gcodeShader_)
        return;
    // 1) Use the G-code shader, set its uniforms:
    gcodeShader_->use();
    glm::mat4 modelMat = glm::mat4(1.0f);  // G-code is already in world space (0,0,0 is bed origin).
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);

    // 2) Delegate to GCodeModel to bind its VAO and draw that layer’s lines:
    if (!gcodeModel_->DrawLayer(layerIndex, *gcodeShader_)) {
        // Layer invalid or empty: nothing to draw
    }
}

// ─── RenderGCodeUpToLayer ────────────────────────────────────────────────────────
// Draw all layers 0..maxLayerIndex. If maxLayerIndex<0, draw everything.
void SceneRenderer::RenderGCodeUpToLayer(int maxLayerIndex) {
    if (!gcodeModel_ || !gcodeShader_)
        return;
    // 1) Use the G-code shader and set uniforms
    gcodeShader_->use();
    glm::mat4 modelMat = glm::mat4(1.0f);
    gcodeShader_->setMat4("model", modelMat);
    gcodeShader_->setMat4("view", viewMatrix_);
    gcodeShader_->setMat4("projection", projectionMatrix_);

    // 2) Instruct GCodeModel to draw up to that layer:
    gcodeModel_->DrawUpToLayer(maxLayerIndex, *gcodeShader_);
}
