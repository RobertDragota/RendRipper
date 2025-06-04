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
#include <mutex>
#include "GizmoController.h"
#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"



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

    std::mutex      pendingMutex_;
    std::string     pendingModelPath_;
    std::atomic<bool> pendingModelReady_{false};

    std::atomic<bool>   generating_{false};
    std::atomic<bool>   generationDone_{false};
    std::string         generationMessage_;        // e.g. “Running Python…”
    std::mutex          generationMessageMutex_;
    std::atomic<float> progress_{0.0f};

    std::unique_ptr<SceneRenderer> renderer_;
    GizmoController                gizmo_;

    float cameraYaw_        = 45.0f;
    float cameraPitch_      = 30.0f;
    float cameraDistance_   = 20.0f;

    int activeModel_ = -1;
    bool showWinDialog  = false;

    bool showErrorModal_ = false;
    std::string errorModalMessage_;

    // Camera: Z-up orbital
    void cameraView(glm::mat4 &view, glm::vec3 &cameraWorldPosition) const;

    // UI
    void showMenuBar();
    void openFileDialog( const std::function<void(std::string&)>& onFileSelected = nullptr );
    void openRenderScene();
    void openModelPropertiesDialog();

    void loadModel( std::string&  modelPath );
    void loadImageFor3DModel(std::string&  imagePath);

    // Interaction
    void getActiveModel(glm::mat4 &viewMatrix, const ImVec2& viewportScreenPos, const ImVec2& viewportSize);
    void renderModels(glm::mat4 &viewMatrix); // Renders models to the current rendertarget (FBO)
    void UnloadModel(int modelIndex);

    void EnforceGridConstraint(int modelIndex);


    void showGenerationModal();

    void showErrorModal(std::string &message);
};