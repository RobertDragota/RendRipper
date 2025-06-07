#pragma once

#include <memory>
#include "CameraController.h"
#include "ModelManager.h"
#include "SceneRenderer.h"
#include "GizmoController.h"
#include "WindowManager.h"
#include "UIManager.h"

class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();
    void Run();

private:
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
