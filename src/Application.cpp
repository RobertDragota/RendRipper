#include "Application.h"
#include <iostream>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"

static void glfw_error_callback(int e, const char* d) {
    std::cerr << "GLFW Error " << e << ": " << d << std::endl;
}

Application::Application(int w, int h, const char* title)
        : width_(w), height_(h)
{
    InitGLFW();
    InitWindow(title);
    InitGLAD();

    shader_   = std::make_unique<Shader>(
            "../resources/shaders/shader.vert",
            "../resources/shaders/shader.frag");
    model_    = std::make_unique<Model>(
            "../resources/models/viking_room.obj");

    InitImGui();

    renderer_ = std::make_unique<SceneRenderer>();
}

Application::~Application() {
    Cleanup();
}

void Application::Run() {
    MainLoop();
}

void Application::InitGLFW() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        throw std::runtime_error("Failed to init GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void Application::InitWindow(const char* title) {
    window_ = glfwCreateWindow(width_, height_, title, nullptr, nullptr);
    if (!window_)
        throw std::runtime_error("Failed to create GLFW window");
    glfwMakeContextCurrent(window_);
}

void Application::InitGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to init GLAD");
    glEnable(GL_DEPTH_TEST);
}

void Application::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable
                      | ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::StyleColorsDark();
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        // DockSpace UI
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGuiWindowFlags wf = ImGuiWindowFlags_NoTitleBar
                              | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockHost", nullptr, wf);
        ImGui::PopStyleVar(2);
        ImGuiID dockID = ImGui::GetID("MyDock");
        ImGui::DockSpace(dockID, ImVec2(0,0),
                         ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        // Control Panel
        ImGui::Begin("Slicer");
        ImGui::SliderFloat3("Translation",
                            glm::value_ptr(transform_.translation),
                            -5.0f, 5.0f);
        ImGui::SliderFloat3("Rotation",
                            glm::value_ptr(transform_.rotation),
                            -180.0f, 180.0f);
        ImGui::SliderFloat("Scale", &transform_.scale,
                           0.1f, 5.0f);
        ImGui::End();

        // Scene + Gizmo in same window
        renderer_->BeginImGuiScene();
        renderer_->Render(*model_, *shader_, transform_);
        gizmo_.Manipulate(renderer_->GetViewMatrix(),
                          renderer_->GetProjectionMatrix(),
                          transform_);
        renderer_->EndImGuiScene();

        // Render ImGui
        ImGui::Render();
        int dw, dh;
        glfwGetFramebufferSize(window_, &dw, &dh);
        glViewport(0,0,dw,dh);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }
        glfwSwapBuffers(window_);
    }
}

void Application::Cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}
