#pragma once

#include <memory>
#include "CameraController.h"
#include "ModelManager.h"
#include "SceneRenderer.h"
#include "GizmoController.h"
#include "WindowManager.h"
#include "UIManager.h"

/**
 * @brief Main application class responsible for running the 3D slicer UI.
 */
class Application {
public:
    /**
     * @brief Construct a new Application instance.
     * @param width Width of the created window.
     * @param height Height of the created window.
     * @param title Title of the application window.
     */
    Application(int width, int height, const char* title);

    /**
     * @brief Destroy the Application and cleanup resources.
     */
    ~Application();

    /**
     * @brief Start the main application loop.
     */
    void Run();

private:
    /**
     * @brief Release window and renderer resources.
     */
    void Cleanup();

    int width_  = 1280;
    int height_ = 720;
    std::unique_ptr<WindowManager> window_;
    std::unique_ptr<SceneRenderer> renderer_;
    ModelManager     modelManager_;
    GizmoController  gizmo_;
    CameraController camera_;
    UIManager        ui_;
};
