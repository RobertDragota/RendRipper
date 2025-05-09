// Application.cpp
#include "Application.h"
#include <iostream>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <Windows.h>
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"
#include "ImGuiFileDialog.h"


static void glfw_error_callback(int error, const char *desc) {
    std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
}

static void framebuffer_size_callback(GLFWwindow *window, int w, int h) {
    glViewport(0, 0, w, h);
}

Application::Application(int w, int h, const char *title)
        : width_(w), height_(h),
          cameraDistance_(5.0f), cameraPitch_(0.0f), cameraYaw_(45.0f) {
    InitGLFW();
    InitWindow(title);
    InitGLAD();

    shader_ = std::make_unique<Shader>(
            "../resources/shaders/shader.vert",
            "../resources/shaders/shader.frag");
    model_ = std::make_unique<Model>(
            "../resources/models/viking_room.obj");
    modelB_ = nullptr;

    // initial offset for model B
    transformB_.translation = glm::vec3(0.0f, -1.0f, 0.0f);
    transformB_.rotationQuat = glm::quat(1, 0, 0, 0);
    transformB_.scale = glm::vec3(1.0f);

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
}

void Application::InitWindow(const char *title) {
    window_ = glfwCreateWindow(width_, height_, title, nullptr, nullptr);
    if (!window_)
        throw std::runtime_error("Failed to create GLFW window");
    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
}

void Application::InitGLAD() {
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        throw std::runtime_error("Failed to init GLAD");
    glEnable(GL_DEPTH_TEST);
}

void Application::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable
                      | ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::StyleColorsDark();
}


static bool RayIntersectSphere(
        const glm::vec3 &ori,        // ray origin
        const glm::vec3 &dir,        // ray direction (normalized)
        const glm::vec3 &center,     // sphere center
        float radius,                // sphere radius
        float &tOut                  // distance along ray
) {
    glm::vec3 L = center - ori;
    float tca = glm::dot(L, dir);
    if (tca < 0.0f) return false;
    float d2 = glm::dot(L, L) - tca * tca;
    if (d2 > radius * radius) return false;
    float thc = std::sqrt(radius * radius - d2);
    tOut = tca - thc;
    return true;
}


void Application::MainLoop() {
    static int activeModel = 0;  // 0 = A, 1 = B

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();


        // compute camera offset
        glm::vec3 offset;
        offset.x = cameraDistance_ * cos(glm::radians(cameraPitch_))
                   * sin(glm::radians(cameraYaw_));
        offset.y = cameraDistance_ * sin(glm::radians(cameraPitch_));
        offset.z = cameraDistance_ * cos(glm::radians(cameraPitch_))
                   * cos(glm::radians(cameraYaw_));

        // pivot follows the active model’s position
        glm::mat4 view = glm::lookAt(
                model_->center + offset,
                model_->center,
                glm::vec3(-1, 0, 0));


        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open .obj…"))
                    showWinDialog = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

// 2) When triggered, configure and invoke the Win32 dialog
        if (showWinDialog) {
            OPENFILENAMEA ofn{};                               // see docs on OPENFILENAMEA :contentReference[oaicite:0]{index=0}
            char szFile[MAX_PATH] = { 0 };

            ofn.lStructSize   = sizeof(ofn);
            ofn.hwndOwner     = glfwGetWin32Window(window_);
            ofn.lpstrFile     = szFile;
            ofn.nMaxFile      = sizeof(szFile);
            // double-null-terminated pairs: "description\0pattern;pattern\0…\0\0"
            ofn.lpstrFilter   = "OBJ files\0*.obj;*.OBJ\0All files\0*.*\0";        // filter syntax :contentReference[oaicite:1]{index=1}
            ofn.nFilterIndex  = 1;
            ofn.Flags         = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;            // only allow existing files

            if (GetOpenFileNameA(&ofn)) {                                           // opens modal dialog; returns nonzero on “Open” :contentReference[oaicite:2]{index=2}
                // User picked a file and hit OK:
                std::string path = szFile;
                modelB_     = std::make_unique<Model>(path);
                transform_ = Transform{};
            }
            showWinDialog = false;
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseOBJ")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                modelB_ = std::make_unique<Model>(path);
                transform_ = Transform{};
            }
            ImGuiFileDialog::Instance()->Close();
        }

        // DockSpace UI (unchanged)…
        ImGuiViewport *vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGuiWindowFlags wf = ImGuiWindowFlags_NoTitleBar
                              | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 5.0f);
        ImGui::Begin("DockHost", nullptr, wf);
        ImGui::PopStyleVar(2);
        ImGuiID dockID = ImGui::GetID("MyDock");
        ImGui::DockSpace(dockID, ImVec2(0, 0),
                         ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        // Control Panel: only the active model’s sliders
        ImGui::Begin("Slicer");

        // add ImGui buttons to switch between models
        ImGui::RadioButton("Model A", &activeModel, 0);
        ImGui::RadioButton("Model B", &activeModel, 1);

        if (activeModel == 0) {
            ImGui::Text("Editing Model A");
            ImGui::SliderFloat3("Translation A",
                                glm::value_ptr(transform_.translation), -5.0f, 5.0f);
            glm::vec3 eA = transform_.getEulerAngles();
            if (ImGui::SliderFloat3("Rotation A",
                                    glm::value_ptr(eA), 0.0f, 360.0f))
                transform_.setEulerAngles(eA);
            ImGui::SliderFloat3("Scale A",
                                glm::value_ptr(transform_.scale), 0.1f, 5.0f);
            if (ImGui::Button("Reset A")) {
                transform_.translation = glm::vec3(0.0f);
                transform_.rotationQuat = glm::quat(1, 0, 0, 0);
                transform_.scale = glm::vec3(1.0f);
            }
        } else {
            ImGui::Text("Editing Model B");
            ImGui::SliderFloat3("Translation B",
                                glm::value_ptr(transformB_.translation), -5.0f, 5.0f);
            glm::vec3 eB = transformB_.getEulerAngles();
            if (ImGui::SliderFloat3("Rotation B",
                                    glm::value_ptr(eB), 0.0f, 360.0f))
                transformB_.setEulerAngles(eB);
            ImGui::SliderFloat3("Scale B",
                                glm::value_ptr(transformB_.scale), 0.1f, 5.0f);
            if (ImGui::Button("Reset B")) {
                transformB_.translation = glm::vec3(0.0f);
                transformB_.rotationQuat = glm::quat(1, 0, 0, 0);
                transformB_.scale = glm::vec3(1.0f);
            }
        }
        ImGui::End();

        // **multi-model draw into FBO**
        renderer_->BeginScene(view);



        // ** click‐picking logic **
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            // 1) mouse pos relative to Scene content
            ImVec2 mp = ImGui::GetMousePos();
            ImVec2 wp = ImGui::GetWindowPos();
            ImVec2 pad = ImGui::GetStyle().WindowPadding;
            float w = ImGui::GetContentRegionAvail().x;
            float h = ImGui::GetContentRegionAvail().y;
            ImVec2 cp{mp.x - (wp.x + pad.x), mp.y - (wp.y + pad.y)};

            // 2) normalized device coords
            float ndcX = (2.0f * cp.x) / w - 1.0f;
            float ndcY = 1.0f - (2.0f * cp.y) / h;

            // 3) unproject into world‐space ray
            glm::vec4 rayClip{ndcX, ndcY, -1.0f, 1.0f};
            glm::vec4 rayEye = glm::inverse(renderer_->GetProjectionMatrix()) * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
            glm::vec3 rayDir = glm::normalize(
                    glm::vec3(glm::inverse(view) * rayEye)
            );

            // 4) sphere tests against each model
            float tA, tB;
            bool hitA = RayIntersectSphere(
                    model_->center + offset, rayDir,
                    model_->center + transform_.translation,
                    model_->radius * transform_.scale.x,
                    tA
            );
            bool hitB = !modelB_ ? false : RayIntersectSphere(
                    model_->center + offset, rayDir,
                    modelB_->center + transformB_.translation,
                    modelB_->radius * transformB_.scale.x,
                    tB
            );

            // choose the nearest hit
            if (hitA && (!hitB || tA < tB)) activeModel = 0;
            else if (hitB) activeModel = 1;
        }


        renderer_->RenderModel(*model_, *shader_, transform_);
        if(modelB_)
        renderer_->RenderModel(*modelB_, *shader_, transformB_);
        // single gizmo on the active one:
        if (activeModel == 0) {
            gizmo_.Manipulate(
                    renderer_->GetViewMatrix(),
                    renderer_->GetProjectionMatrix(),
                    transform_);
        } else if(modelB_) {
            gizmo_.Manipulate(
                    renderer_->GetViewMatrix(),
                    renderer_->GetProjectionMatrix(),
                    transformB_);
        }
        renderer_->EndScene();

        // Render ImGui + swap
        ImGui::Render();
        int dw, dh;
        glfwGetFramebufferSize(window_, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup = glfwGetCurrentContext();
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
