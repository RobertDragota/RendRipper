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
#include <imgui.h>
#include <mutex>
#include <filesystem>
#include "GizmoController.h"
#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"
#include "MeshRepairer.h"


class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();
    void Run();

private:
    void InitGLFW();
    void InitWindow(const char* title);
    void InitGLAD() const;
    void InitImGui() const;
    void MainLoop();

    void Cleanup();

    GLFWwindow*                    window_    = nullptr;
    int                            width_     = 1280;
    int                            height_    = 720;


    std::shared_ptr<GCodeModel> gcodeModel_;
    int currentGCodeLayer_ = -1;

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

    // Slicing state
    std::atomic<bool>   slicing_{false};
    std::atomic<bool>   slicingDone_{false};
    std::string         slicingMessage_;
    std::mutex          slicingMessageMutex_;
    std::atomic<float>  slicingProgress_{0.0f};

    std::vector<std::string> loadedModelPaths_;

    std::unique_ptr<SceneRenderer> renderer_;
    GizmoController                gizmo_;

    Transform                        bedCornerGizmoTransform_;

    bool showWireframe_ = false;
    bool useOrtho_      = false;

    float cameraYaw_        = 0.0f;
    float cameraPitch_      = 30.0f;
    float cameraDistance_   = 500.0f;

    int activeModel_ = -1;
    bool showWinDialog  = false;

    bool showErrorModal_ = false;
    std::string errorModalMessage_;
    std::vector<glm::vec3> loadedMeshDimensions_;
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
    void showSlicingModal();

    void sliceActiveModel();

    void showErrorModal(std::string &message);
};