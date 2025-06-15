#pragma once
#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <imgui.h>
#include <ImGuizmo.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include "ModelManager.h"
#include "SceneRenderer.h"
#include "GizmoController.h"
#include "CameraController.h"
#include "GCodeModel.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Manages all ImGui windows and user interactions.
 */
class UIManager
{
public:
    /** Construct the UI manager with required subsystems. */
    UIManager
    (
        ModelManager &mm, SceneRenderer *renderer,
        GizmoController &gizmo, CameraController &camera,
        GLFWwindow *window
    );

    /// Render one frame of the UI.
    void Frame();

private:
    void showMenuBar();

    /** Show a platform file dialog and invoke callback on selection. */
    void openFileDialog(const std::function<void(std::string &)> &onFileSelected = nullptr);

    void openRenderScene();

    void openModelPropertiesDialog();

    /** Load persistent model settings from disk. */
    void loadModelSettings();

    /** Write model settings back to disk. */
    void saveModelSettings();

    /** Load a 3D model from the given file path. */
    void loadModel(std::string &modelPath);

    /** Run the TripoSR pipeline to generate a model from an image. */
    void loadImageFor3DModel(std::string &imagePath);

    /** Kick off slicing of the currently active model. */
    void sliceActiveModel();

    /** Remove a model from the manager. */
    void UnloadModel(int idx);

    /** Load the resulting G-code after slicing completes. */
    void finalizeSlicing();

    /** Display progress modal during model generation. */
    void showGenerationModal();

    /** Display progress modal during slicing. */
    void showSlicingModal();

    /** Display an error popup with the given message. */
    void showErrorModal(std::string &message);

    /** Determine which model is under the mouse cursor. */
    void getActiveModel(glm::mat4 &viewMatrix, const ImVec2 &viewportScreenPos, const ImVec2 &viewportSize);

    /** Render all loaded models. */
    void renderModels(glm::mat4 &viewMatrix);

    /** Handle mouse and keyboard input inside the viewport. */
    void handleViewportInput
    (
        glm::mat4 &viewMatrix,
        const ImVec2 &viewportPos,
        const ImVec2 &viewportSize
    );

    ModelManager &modelManager_;
    SceneRenderer *renderer_;
    GizmoController &gizmo_;
    CameraController &camera_;

    std::shared_ptr<GCodeModel> gcodeModel_;
    int currentGCodeLayer_ = -1;

    std::atomic<bool> generating_{false};
    std::atomic<bool> generationDone_{false};
    std::string generationMessage_;
    std::mutex generationMessageMutex_;
    std::atomic<float> progress_{0.0f};

    std::atomic<bool> slicing_{false};
    std::atomic<bool> slicingDone_{false};
    std::string slicingMessage_;
    std::mutex slicingMessageMutex_;
    std::atomic<float> slicingProgress_{0.0f};

    bool showWireframe_ = false;
    bool useOrtho_ = false;

    int activeModel_ = -1;
    bool showWinDialog = false;
    bool showErrorModal_ = false;
    std::string errorModalMessage_;
    GLFWwindow *window_ = nullptr;

    // Slicing bookkeeping
    std::string pendingGcodePath_;
    std::string pendingResizedPath_;
    std::string pendingStlPath_;
    int slicingModelIndex_ = -1;
    std::atomic<bool> loadGcodePending_{false};

    json modelSettings_;
    bool modelSettingsLoaded_ = false;
    std::unordered_map<std::string, std::vector<std::string> > enumOptions_;
};
