// Application.cpp
#include "Application.h"
#include <iostream>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <windows.h>
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

    plateShader_ = std::make_unique<Shader>(
            "../resources/shaders/plate_shader.vert",
            "../resources/shaders/plate_shader.frag");
    plateModel_ = std::make_unique<Model>(
            "../resources/models/printing_plate_uv.obj");


    plateTransform_ = std::make_unique<Transform>();
    plateTransform_->translation = glm::vec3(0.0f, -1.0f, 0.0f);
    plateTransform_->rotationQuat = glm::quat(1, 0, 0, 0);
    plateTransform_->scale = glm::vec3(1.0f);

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

void Application::cameraView(glm::mat4 &view, glm::vec3 &offset) const {


    offset.x = cameraDistance_ * cos(glm::radians(cameraPitch_))
               * sin(glm::radians(cameraYaw_));
    offset.y = cameraDistance_ * sin(glm::radians(cameraPitch_));
    offset.z = cameraDistance_ * cos(glm::radians(cameraPitch_))
               * cos(glm::radians(cameraYaw_));

    auto pivot = glm::vec3(0.0f);


    view = glm::lookAt(
            pivot + offset,
            pivot,
            glm::vec3(0, 1, 0)
    );
}

void Application::showMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open file"))
                showWinDialog = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Application::openFileDialog() {

    OPENFILENAMEA ofn{};
    char szFile[MAX_PATH] = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = glfwGetWin32Window(window_);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);

    ofn.lpstrFilter = "OBJ files\0*.obj;*.OBJ\0All files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {

        std::string path = szFile;

        std::unique_ptr<Transform> transform = std::make_unique<Transform>();
        transform->translation = glm::vec3(0.0f, -1.0f, 0.0f);
        transform->rotationQuat = glm::quat(1, 0, 0, 0);
        transform->scale = glm::vec3(1.0f);

        modelTransformations_.emplace_back(std::move(transform));


        modelShaders_.emplace_back(
                std::make_unique<Shader>(
                        "../../resources/shaders/model_shader.vert",
                        "../../resources/shaders/model_shader.frag"));


        models_.emplace_back(std::make_unique<Model>(path));

        std::cout << "Model loaded: " << path << std::endl;
    }
    showWinDialog = false;

}

void Application::openRenderScene() {

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

}

void Application::openModelPropertiesDialog() {
    ImGui::Begin("Slicer");

    if (!modelTransformations_.empty() && activeModel_ < modelTransformations_.size()) {
        ImGui::Text("Editing Model");
        ImGui::SliderFloat3("Translation",
                            glm::value_ptr(modelTransformations_[activeModel_]->translation), -5.0f, 5.0f);
        glm::vec3 eA = modelTransformations_[activeModel_]->getEulerAngles();
        if (ImGui::SliderFloat3("Rotation",
                                glm::value_ptr(eA), 0.0f, 360.0f))
            modelTransformations_[activeModel_]->setEulerAngles(eA);
        ImGui::SliderFloat3("Scale",
                            glm::value_ptr(modelTransformations_[activeModel_]->scale), 0.1f, 5.0f);
        if (ImGui::Button("Reset")) {
            modelTransformations_[activeModel_]->translation = glm::vec3(0.0f);
            modelTransformations_[activeModel_]->rotationQuat = glm::quat(1, 0, 0, 0);
            modelTransformations_[activeModel_]->scale = glm::vec3(1.0f);
        }
    }else{
        ImGui::Text("Printing Plate");
        ImGui::SliderFloat3("Translation",
                            glm::value_ptr(plateTransform_->translation), -5.0f, 5.0f);
        glm::vec3 eA = plateTransform_->getEulerAngles();
        if (ImGui::SliderFloat3("Rotation",
                                glm::value_ptr(eA), 0.0f, 360.0f))
            plateTransform_->setEulerAngles(eA);
        ImGui::SliderFloat3("Scale",
                            glm::value_ptr(plateTransform_->scale), 0.1f, 5.0f);
        if (ImGui::Button("Reset")) {
            plateTransform_->translation = glm::vec3(0.0f);
            plateTransform_->rotationQuat = glm::quat(1, 0, 0, 0);
            plateTransform_->scale = glm::vec3(1.0f);
        }
    }
    ImGui::End();
}

void Application::getActiveModel(glm::mat4 &view) {

    float nearestT = std::numeric_limits<float>::infinity();
    int pickedIndex = -1;

    ImVec2 mp = ImGui::GetMousePos();
    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 pad = ImGui::GetStyle().WindowPadding;
    float w = ImGui::GetContentRegionAvail().x;
    float h = ImGui::GetContentRegionAvail().y;
    ImVec2 cp{mp.x - (wp.x + pad.x), mp.y - (wp.y + pad.y)};

    float ndcX = (2.0f * cp.x) / w - 1.0f;
    float ndcY = 1.0f - (2.0f * cp.y) / h;

    glm::vec4 rayClip{ndcX, ndcY, -1.0f, 1.0f};
    glm::mat4 invProj = glm::inverse(renderer_->GetProjectionMatrix());
    glm::vec4 rayEye = invProj * rayClip;
    rayEye.z = -1.0f;
    rayEye.w = 0.0f;

    glm::mat4 invView = glm::inverse(view);
    glm::vec3 rayDir = glm::normalize(glm::vec3(invView * rayEye));
    glm::vec3 rayOrigin = glm::vec3(invView[3]);

    for (size_t i = 0; i < models_.size(); ++i) {
        float tHit = 0.0f;
        glm::vec3 center = models_[i]->center + modelTransformations_[i]->translation;
        float radius = models_[i]->radius * modelTransformations_[i]->scale.x;
        if (RayIntersectSphere(rayOrigin, rayDir, center, radius, tHit)) {
            if (tHit < nearestT) {
                nearestT = tHit;
                pickedIndex = int(i);
            }
        }
    }

    activeModel_ = pickedIndex;
}

void Application::renderModels(glm::mat4 &view) {


    for (size_t i = 0; i < models_.size(); ++i) {

        renderer_->RenderModel(*models_[i], *modelShaders_[i],
                               *modelTransformations_[i]);


    }
    if (!modelTransformations_.empty() && activeModel_ != -1)
        gizmo_.Manipulate(
                renderer_->GetViewMatrix(),
                renderer_->GetProjectionMatrix(),
                *modelTransformations_[activeModel_]);

}

void Application::renderPrintPlate(glm::mat4 &view) {
    renderer_->RenderModel(*plateModel_, *plateShader_,
                           *plateTransform_);

    if (activeModel_ == -1)
        gizmo_.Manipulate(
                renderer_->GetViewMatrix(),
                renderer_->GetProjectionMatrix(),
                *plateTransform_);

}

void Application::MainLoop() {

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        glm::mat4 view;
        glm::vec3 offset;

        cameraView(view, offset);

        showMenuBar();

        if (showWinDialog) {
            openFileDialog();
        }


        openRenderScene();

        openModelPropertiesDialog();

        renderer_->BeginScene(view);

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            getActiveModel(view);
        }

        renderPrintPlate(view);

        renderModels(view);

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
