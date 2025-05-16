#pragma once

#include <glad/glad.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include "GLFW/glfw3native.h"
#include <memory>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
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

    // window properties
    GLFWwindow*                    window_    = nullptr;
    int                            width_     = 0, height_ = 0;

    // models and shaders
    std::vector<std::unique_ptr<Shader>>        modelShaders_;
    std::vector<std::unique_ptr<Model>>         models_;
    std::vector<std::unique_ptr<Transform>>     modelTransformations_;

    std::unique_ptr<Shader>                   plateShader_;
    std::unique_ptr<Model>                    plateModel_;
    std::unique_ptr<Transform>                plateTransform_;

    std::unique_ptr<SceneRenderer> renderer_;

    GizmoController                gizmo_;

    float cameraYaw_ ;
    float cameraPitch_ ;
    float cameraDistance_ ;

    int activeModel_ = -1;

    bool showWinDialog  = false;

    void cameraView(glm::mat4 &view, glm::vec3 &offset) const;

    void showMenuBar();

    void openFileDialog();

    void openRenderScene();

    void openModelPropertiesDialog();

    void getActiveModel( glm::mat4 &view);

    void renderModels(glm::mat4 &view);

};
