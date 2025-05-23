#pragma once

#include <glad/glad.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <GLFW/glfw3native.h>
#endif

#include <memory>
#include <string>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <imgui.h> // For ImVec2
#include "GizmoController.h"
#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"

// Ensure Axis enum is accessible, e.g., by including Model.h or defining it here/elsewhere
// enum class Axis { X_UP, Y_UP, Z_UP, UNKNOWN }; // Example


class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();
    void Run();

private:
    void InitGLFW();
    void InitWindow(const char* title);
    void InitGLAD();
    void InitImGui();
    void MainLoop();
    void Cleanup();

    GLFWwindow*                    window_    = nullptr;
    int                            width_     = 1280;
    int                            height_    = 720;

    std::vector<std::unique_ptr<Shader>>        modelShaders_;
    std::vector<std::unique_ptr<Model>>         models_;
    std::vector<std::unique_ptr<Transform>>     modelTransformations_;

    std::unique_ptr<SceneRenderer> renderer_;
    GizmoController                gizmo_;

    float cameraYaw_        = 45.0f;
    float cameraPitch_      = 30.0f;
    float cameraDistance_   = 20.0f;

    int activeModel_ = -1;
    bool showWinDialog  = false;

    // Camera: Z-up orbital
    void cameraView(glm::mat4 &view, glm::vec3 &cameraWorldPosition) const;

    // UI
    void showMenuBar();
    void openFileDialog();
    void openRenderScene(); // Dockspace setup
    void openModelPropertiesDialog();

    // Interaction
    void getActiveModel(glm::mat4 &viewMatrix, const ImVec2& viewportScreenPos, const ImVec2& viewportSize);
    void renderModels(glm::mat4 &viewMatrix); // Renders models to the current rendertarget (FBO)
    void UnloadModel(int modelIndex);

    void EnforceGridConstraint(int modelIndex);
};