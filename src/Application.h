#pragma once

#include <glad/glad.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include "GLFW/glfw3native.h"
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "SceneRenderer.h"
#include "GizmoController.h"
#include "Shader.h"
#include "Model.h"


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
    int                            width_     = 0, height_ = 0;
    std::unique_ptr<Shader>        shader_;
    std::unique_ptr<Model>         model_;
    std::unique_ptr<Model>         modelB_;
    std::unique_ptr<SceneRenderer> renderer_;   // creat după GLAD
    GizmoController                gizmo_;
    GizmoController                gizmoB_;
    Transform                      transform_;
    Transform                      transformB_;
    float cameraYaw_ ;
    float cameraPitch_ ;
    float cameraDistance_ ;

    bool showWinDialog  = false;
};
