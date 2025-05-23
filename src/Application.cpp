#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <cmath> // For std::abs, fmod, cos, sin

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"      // Should define Model, Transform, Axis enum
#include "Transform.h"  // If Transform is in its own file
#include "GizmoController.h"

#ifdef _WIN32

#include <windows.h>

#endif

// Helper struct for Z bounds
struct WorldMinMaxZ {
    float minZ;
    float maxZ;
};

// Helper function to calculate world Z bounds
WorldMinMaxZ CalculateWorldMinMaxZ(const Model &model, const Transform &transform) {
    glm::vec3 localCorners[8] = {
            {model.minBounds.x, model.minBounds.y, model.minBounds.z},
            {model.maxBounds.x, model.minBounds.y, model.minBounds.z},
            {model.minBounds.x, model.maxBounds.y, model.minBounds.z},
            {model.maxBounds.x, model.maxBounds.y, model.minBounds.z},
            {model.minBounds.x, model.minBounds.y, model.maxBounds.z},
            {model.maxBounds.x, model.minBounds.y, model.maxBounds.z},
            {model.minBounds.x, model.maxBounds.y, model.maxBounds.z},
            {model.maxBounds.x, model.maxBounds.y, model.maxBounds.z}
    };
    glm::mat4 modelMatrix = transform.getMatrix();
    WorldMinMaxZ result = {std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
    for (int i = 0; i < 8; ++i) {
        glm::vec3 worldCorner = glm::vec3(modelMatrix * glm::vec4(localCorners[i], 1.0f));
        result.minZ = glm::min(result.minZ, worldCorner.z);
        result.maxZ = glm::max(result.maxZ, worldCorner.z);
    }
    return result;
}


void ApplyModernDarkStyle() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;
    ImVec4 bg_main_very_dark = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    ImVec4 bg_main = ImVec4(0.118f, 0.118f, 0.118f, 1.00f); // #1E1E1E
    ImVec4 bg_secondary = ImVec4(0.145f, 0.145f, 0.149f, 1.00f); // #252526
    ImVec4 bg_widget = ImVec4(0.200f, 0.200f, 0.200f, 1.00f); // #333333
    ImVec4 text_main = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);   // #DCDCDC
    ImVec4 text_disabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    ImVec4 accent_main = ImVec4(0.000f, 0.478f, 0.800f, 1.00f); // #007ACC
    ImVec4 accent_hover = ImVec4(0.100f, 0.578f, 0.900f, 1.00f);
    ImVec4 accent_active = ImVec4(0.000f, 0.361f, 0.600f, 1.00f);
    ImVec4 border_main = ImVec4(0.220f, 0.220f, 0.220f, 1.00f); // #383838
    ImVec4 border_light = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.ScrollbarSize = 15.0f;
    style.GrabMinSize = 12.0f;
    style.IndentSpacing = 20.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.WindowRounding = 5.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    colors[ImGuiCol_Text] = text_main;
    colors[ImGuiCol_TextDisabled] = text_disabled;
    colors[ImGuiCol_WindowBg] = bg_main;
    colors[ImGuiCol_ChildBg] = bg_secondary;
    colors[ImGuiCol_PopupBg] = bg_secondary;
    colors[ImGuiCol_Border] = border_main;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = bg_widget;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(bg_widget.x * 1.3f, bg_widget.y * 1.3f, bg_widget.z * 1.3f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(bg_widget.x * 0.9f, bg_widget.y * 0.9f, bg_widget.z * 0.9f, 1.00f);
    colors[ImGuiCol_TitleBg] = bg_main_very_dark;
    colors[ImGuiCol_TitleBgActive] = accent_active;
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(bg_main_very_dark.x, bg_main_very_dark.y, bg_main_very_dark.z, 0.75f);
    colors[ImGuiCol_MenuBarBg] = bg_secondary;
    colors[ImGuiCol_ScrollbarBg] = ImVec4(bg_widget.x, bg_widget.y, bg_widget.z, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_CheckMark] = accent_main;
    colors[ImGuiCol_SliderGrab] = accent_main;
    colors[ImGuiCol_SliderGrabActive] = accent_hover;
    colors[ImGuiCol_Button] = accent_main;
    colors[ImGuiCol_ButtonHovered] = accent_hover;
    colors[ImGuiCol_ButtonActive] = accent_active;
    colors[ImGuiCol_Header] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.31f);
    colors[ImGuiCol_HeaderHovered] = accent_hover;
    colors[ImGuiCol_HeaderActive] = accent_main;
    colors[ImGuiCol_Separator] = border_light;
    colors[ImGuiCol_SeparatorHovered] = ImVec4(border_light.x * 1.2f, border_light.y * 1.2f, border_light.z * 1.2f,
                                               1.0f);
    colors[ImGuiCol_SeparatorActive] = accent_main;
    colors[ImGuiCol_ResizeGrip] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.78f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(bg_secondary.x * 0.9f, bg_secondary.y * 0.9f, bg_secondary.z * 0.9f, 0.9f);
    colors[ImGuiCol_TabHovered] = accent_hover;
    colors[ImGuiCol_TabActive] = accent_main;
    colors[ImGuiCol_TabUnfocused] = ImVec4(bg_secondary.x * 0.8f, bg_secondary.y * 0.8f, bg_secondary.z * 0.8f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(accent_main.x * 0.8f, accent_main.y * 0.8f, accent_main.z * 0.8f,
                                                 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.7f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = accent_main;
    colors[ImGuiCol_PlotLinesHovered] = accent_hover;
    colors[ImGuiCol_PlotHistogram] = accent_main;
    colors[ImGuiCol_PlotHistogramHovered] = accent_hover;
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(accent_active.x, accent_active.y, accent_active.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.90f);
    colors[ImGuiCol_NavHighlight] = accent_main;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(bg_main.x, bg_main.y, bg_main.z, 0.35f);
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;
}

static void glfw_error_callback(int error, const char *desc) { /* ... */ std::cerr << "GLFW Error " << error << ": "
                                                                                   << desc << std::endl;
}

static void framebuffer_size_callback(GLFWwindow * /*window*/, int w, int h) { /* ... */ glViewport(0, 0, w, h); }

Application::Application(int w, int h, const char *title)
        : width_(w), height_(h), activeModel_(-1), showWinDialog(false) {
    InitGLFW();
    InitWindow(title);
    InitGLAD();
    InitImGui();
    try {
        renderer_ = std::make_unique<SceneRenderer>();
        if (renderer_) { renderer_->SetViewportSize(width_, height_); }
    } catch (const std::exception &e) {
        std::cerr << "Failed to init SceneRenderer: " << e.what() << std::endl;
        throw;
    }
}

Application::~Application() { Cleanup(); }

void Application::Run() { MainLoop(); }

void Application::InitGLFW() { /* ... as before ... */
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) throw std::runtime_error("Failed to init GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void Application::InitWindow(const char *title) { /* ... as before ... */
    window_ = glfwCreateWindow(width_, height_, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("No GLFW window");
    }
    glfwSetWindowUserPointer(window_, this);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    glfwSetCursorPosCallback(window_, [](GLFWwindow *w, double x, double y) {
        Application *app = static_cast<Application *>(glfwGetWindowUserPointer(w));
        if (!app)return;
        static double lx = x, ly = y;
        static bool fm = true;
        if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (ImGui::GetIO().WantCaptureMouse) {
                fm = true;
                return;
            }
            if (fm) {
                lx = x;
                ly = y;
                fm = false;
            }
            double ox = x - lx, oy = ly - y;
            lx = x;
            ly = y;
            float s = 0.2f;
            ox *= s;
            oy *= s;
            app->cameraYaw_ += static_cast<float>(ox);
            app->cameraPitch_ += static_cast<float>(oy);
            app->cameraPitch_ = glm::clamp(app->cameraPitch_, -89.f, 89.f);
        } else fm = true;
    });
    glfwSetScrollCallback(window_, [](GLFWwindow *w, double, double yo) {
        Application *app = static_cast<Application *>(glfwGetWindowUserPointer(w));
        if (!app || ImGui::GetIO().WantCaptureMouse)return;
        float zs = 0.8f;
        app->cameraDistance_ -= static_cast<float>(yo) * zs;
        app->cameraDistance_ = glm::clamp(app->cameraDistance_, 2.f, 150.f);
    });
}

void Application::InitGLAD() { /* ... as before ... */
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        if (window_)glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("No GLAD");
    }
    glEnable(GL_DEPTH_TEST);
}

void Application::InitImGui() { /* ... as before ... */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ApplyModernDarkStyle();
    ImGuiStyle &s = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        s.WindowRounding = 0;
        s.Colors[ImGuiCol_WindowBg].w = 1;
    }
    std::string fp = "../../assets/fonts/Orbitron-Bold.ttf";
    if (!io.Fonts->AddFontFromFileTTF(fp.c_str(), 16.f))std::cerr << "Warn:FontFail " << fp << std::endl;
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

static bool RayIntersectSphere(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 center, float radius,
                               float &tOut) { /* ... */
    glm::dvec3 O = glm::dvec3(orig) - glm::dvec3(center);
    glm::dvec3 D = glm::dvec3(dir);
    double a = glm::dot(D, D), b = 2. * glm::dot(D, O), c = glm::dot(O, O) - (double) radius * radius;
    double disc = b * b - 4. * a * c;
    if (disc < 0.)return false;
    double sD = std::sqrt(disc);
    double q = -0.5 * (b + (b > 0. ? sD : -sD));
    double t0 = q / a, t1 = c / q;
    if (t0 > t1)std::swap(t0, t1);
    if (t1 < 0.)return false;
    tOut = (t0 < 0.) ? static_cast<float>(t1) : static_cast<float>(t0);
    return true;
}

void Application::cameraView(glm::mat4 &view, glm::vec3 &cameraWorldPosition) const { /* ... as before (Z-up) ... */
    float rP = glm::radians(cameraPitch_), rY = glm::radians(cameraYaw_);
    cameraWorldPosition.x = cameraDistance_ * cos(rP) * cos(rY);
    cameraWorldPosition.y = cameraDistance_ * cos(rP) * sin(rY);
    cameraWorldPosition.z = cameraDistance_ * sin(rP);
    glm::vec3 piv(0.f), up(0.f, 0.f, 1.f);
    view = glm::lookAt(cameraWorldPosition + piv, piv, up);
}

void Application::showMenuBar() { /* ... as before ... */
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open OBJ Model...", "Ctrl+O"))showWinDialog = true;
            if (ImGui::MenuItem("Exit"))glfwSetWindowShouldClose(window_, true);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (activeModel_ != -1) {
                if (ImGui::MenuItem("Translate", "T", gizmo_.GetCurrentMode() == ImGuizmo::TRANSLATE))
                    gizmo_.SetCurrentMode(ImGuizmo::TRANSLATE);
                if (ImGui::MenuItem("Rotate", "R", gizmo_.GetCurrentMode() == ImGuizmo::ROTATE))
                    gizmo_.SetCurrentMode(ImGuizmo::ROTATE);
                if (ImGui::MenuItem("Scale", "S", gizmo_.GetCurrentMode() == ImGuizmo::SCALE))
                    gizmo_.SetCurrentMode(ImGuizmo::SCALE);
            } else ImGui::TextDisabled("No model selected");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Application::EnforceGridConstraint(int modelIndex) {
    if (modelIndex < 0 || modelIndex >= static_cast<int>(models_.size())) return;

    Model &model = *models_[modelIndex];
    Transform &transform = *modelTransformations_[modelIndex];

    WorldMinMaxZ worldZ = CalculateWorldMinMaxZ(model, transform);
    float groundPlaneZ = 0.0f;

    if (worldZ.minZ < groundPlaneZ) {
        float offsetNeeded = groundPlaneZ - worldZ.minZ;
        transform.translation.z += offsetNeeded;
    }
}

void Application::openFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn{};
    char szFile[MAX_PATH] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = glfwGetWin32Window(window_);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "OBJ Files\0*.obj;*.OBJ\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == TRUE) {
        std::string p = szFile;
        try {
            auto mPtr = std::make_unique<Model>(p);
            mPtr->computeBounds();

            glm::vec3 oC = mPtr->center, oMin = mPtr->minBounds, oMax = mPtr->maxBounds;
            auto t = std::make_unique<Transform>();

            glm::vec3 c[8];
            c[0] = t->rotationQuat * glm::vec3(oMin.x, oMin.y, oMin.z);
            c[1] = t->rotationQuat * glm::vec3(oMax.x, oMin.y, oMin.z);
            c[2] = t->rotationQuat * glm::vec3(oMin.x, oMax.y, oMin.z);
            c[3] = t->rotationQuat * glm::vec3(oMax.x, oMax.y, oMin.z);
            c[4] = t->rotationQuat * glm::vec3(oMin.x, oMin.y, oMax.z);
            c[5] = t->rotationQuat * glm::vec3(oMax.x, oMin.y, oMax.z);
            c[6] = t->rotationQuat * glm::vec3(oMin.x, oMax.y, oMax.z);
            c[7] = t->rotationQuat * glm::vec3(oMax.x, oMax.y, oMax.z);
            glm::vec3 newMin = c[0];
            for (int i = 1; i < 8; ++i)newMin = glm::min(newMin, c[i]);
            glm::vec3 cRot = t->rotationQuat * oC;
            t->translation = glm::vec3(-cRot.x, -cRot.y, -newMin.z);

            modelTransformations_.push_back(std::move(t));
            modelShaders_.emplace_back(std::make_unique<Shader>("../../resources/shaders/model_shader.vert",
                                                                "../../resources/shaders/model_shader.frag"));
            models_.emplace_back(std::move(mPtr));
            activeModel_ = static_cast<int>(models_.size() - 1);
            EnforceGridConstraint(activeModel_); // Enforce constraint after loading
        } catch (const std::exception &e) { std::cerr << "Err load " << p << ": " << e.what() << std::endl; }
    }
#else
    std::cerr<<"File dialog N/A\n";
#endif
    showWinDialog = false;
}

void Application::UnloadModel(int idx) {/*..as before..*/if (idx < 0 || idx >= models_.size())return;
    models_.erase(models_.begin() + idx);
    modelShaders_.erase(modelShaders_.begin() + idx);
    modelTransformations_.erase(modelTransformations_.begin() + idx);
    if (activeModel_ == idx)activeModel_ = -1; else if (activeModel_ > idx)activeModel_--;
}

void Application::openRenderScene() {/*..as before..*/ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags f = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockspaceHost", nullptr, f);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(ImGui::GetID("MyMainDockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void Application::openModelPropertiesDialog() {
    ImGui::Begin("Model Properties");
    if (activeModel_ >= 0 && activeModel_ < static_cast<int>(modelTransformations_.size())) {
        ImGui::Text("Model Index: %d", activeModel_);
        ImGui::Separator();
        auto &tf = *modelTransformations_[activeModel_];
        bool changed = false;
        if (ImGui::DragFloat3("Translation##T", glm::value_ptr(tf.translation), 0.01f)) changed = true;
        glm::vec3 euler = tf.getEulerAngles();
        if (ImGui::DragFloat3("Rotation##R", glm::value_ptr(euler), 0.5f)) {
            tf.setEulerAngles(euler);
            changed = true;
        }
        if (ImGui::DragFloat3("Scale##S", glm::value_ptr(tf.scale), 0.01f, 0.001f, 100.0f)) changed = true;

        if (changed) EnforceGridConstraint(activeModel_);

        ImGui::Separator();
        if (ImGui::Button("Reset Transform")) {
            // ... (Reset transform logic as before) ...
            glm::vec3 oC = models_[activeModel_]->center;
            glm::vec3 oMin = models_[activeModel_]->minBounds;
            glm::vec3 oMax = models_[activeModel_]->maxBounds;

            tf.rotationQuat = glm::quat(1.0f, 0, 0, 0);
            glm::vec3 c[8];
            c[0] = tf.rotationQuat * glm::vec3(oMin.x, oMin.y, oMin.z);
            c[1] = tf.rotationQuat * glm::vec3(oMax.x, oMin.y, oMin.z);
            c[2] = tf.rotationQuat * glm::vec3(oMin.x, oMax.y, oMin.z);
            c[3] = tf.rotationQuat * glm::vec3(oMax.x, oMax.y, oMin.z);
            c[4] = tf.rotationQuat * glm::vec3(oMin.x, oMin.y, oMax.z);
            c[5] = tf.rotationQuat * glm::vec3(oMax.x, oMin.y, oMax.z);
            c[6] = tf.rotationQuat * glm::vec3(oMin.x, oMax.y, oMax.z);
            c[7] = tf.rotationQuat * glm::vec3(oMax.x, oMax.y, oMax.z);
            glm::vec3 newMin = c[0];
            for (int i = 1; i < 8; ++i)newMin = glm::min(newMin, c[i]);
            glm::vec3 centerRot = tf.rotationQuat * oC;
            tf.translation = glm::vec3(-centerRot.x, -centerRot.y, -newMin.z);
            tf.scale = glm::vec3(1.0f);
            EnforceGridConstraint(activeModel_); // Enforce after reset
        }
        ImGui::SameLine();
        if (ImGui::Button("Unload Model")) UnloadModel(activeModel_);
    } else ImGui::Text("No model selected.");
    ImGui::End();
}

void Application::getActiveModel(glm::mat4 &viewMatrix, const ImVec2 &viewportScreenPos,
                                 const ImVec2 &viewportSize) { /* ... as before ... */
    float nearT = std::numeric_limits<float>::infinity();
    int pick = -1;
    ImVec2 mG = ImGui::GetMousePos();
    float rX = mG.x - viewportScreenPos.x, rY = mG.y - viewportScreenPos.y;
    float nX = (2.f * rX) / viewportSize.x - 1.f, nY = 1.f - (2.f * rY) / viewportSize.y;
    if (!renderer_ || viewportSize.x <= 0 || viewportSize.y <= 0)return;
    glm::mat4 iP = glm::inverse(renderer_->GetProjectionMatrix());
    glm::vec4 rE = iP * glm::vec4(nX, nY, -1, 1);
    rE = glm::vec4(rE.x, rE.y, -1, 0);
    glm::mat4 iV = glm::inverse(viewMatrix);
    glm::vec3 rO = glm::vec3(iV[3]), rD = glm::normalize(glm::vec3(iV * rE));
    for (size_t i = 0; i < models_.size(); ++i) {
        if (!models_[i] || !modelTransformations_[i])continue;
        float t;
        glm::mat4 mW = modelTransformations_[i]->getMatrix();
        glm::vec3 wC = glm::vec3(mW * glm::vec4(models_[i]->center, 1));
        float mS = glm::max(modelTransformations_[i]->scale.x,
                            glm::max(modelTransformations_[i]->scale.y, modelTransformations_[i]->scale.z));
        float wR = models_[i]->radius * mS;
        if (RayIntersectSphere(rO, rD, wC, wR, t) && t < nearT) {
            nearT = t;
            pick = static_cast<int>(i);
        }
    }
    activeModel_ = pick;
}

void Application::renderModels(glm::mat4 &/*viewMatrixIgnored*/) { /* ... as before ... */
    for (size_t i = 0; i < models_.size(); ++i) {
        if (models_[i] && modelShaders_[i] && modelTransformations_[i] && renderer_ && modelShaders_[i]->ID != 0) {
            renderer_->RenderModel(*models_[i], *modelShaders_[i], *modelTransformations_[i]);
        }
    }
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        glm::mat4 viewMat;
        glm::vec3 camWorldPos;
        cameraView(viewMat, camWorldPos);
        showMenuBar();
        if (showWinDialog) openFileDialog();
        openRenderScene();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("3D Viewport");
        {
            ImVec2 viewportContentStart = ImGui::GetCursorScreenPos();
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            if (viewportSize.x < 1.0f) viewportSize.x = 1.0f;
            if (viewportSize.y < 1.0f) viewportSize.y = 1.0f;

            if (renderer_) {
                renderer_->SetViewportSize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
                renderer_->BeginScene(viewMat, camWorldPos);
                renderModels(viewMat);
                renderer_->EndScene();
                GLuint texID = renderer_->GetSceneTexture();
                ImGui::Image(texID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
            }

            ImVec2 actualViewportTopLeft = ImGui::GetItemRectMin();

            if (ImGui::IsWindowHovered()) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
                    getActiveModel(viewMat, actualViewportTopLeft, viewportSize);
                }
                // Camera controls specific to when "3D Viewport" is hovered
                // This replaces the global GLFW callbacks for mouse if you prefer viewport-specific control
                // For RMB rotation:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGuizmo::IsUsing()) {
                    ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
                    float sensitivity = 0.2f;
                    cameraYaw_ += delta.x * sensitivity;
                    cameraPitch_ += delta.y * sensitivity;
                    cameraPitch_ = glm::clamp(cameraPitch_, -89.0f, 89.0f);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
                }
                // For Mouse Wheel Zoom:
                ImGuiIO &io = ImGui::GetIO();
                if (io.MouseWheel != 0.0f && !ImGuizmo::IsUsing()) {
                    float zoomSpeed = 0.8f;
                    cameraDistance_ -= io.MouseWheel * zoomSpeed;
                    cameraDistance_ = glm::clamp(cameraDistance_, 2.0f, 150.0f);
                }
            }

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(actualViewportTopLeft.x, actualViewportTopLeft.y, viewportSize.x, viewportSize.y);
            if (activeModel_ >= 0 && activeModel_ < static_cast<int>(models_.size()) && renderer_) {
                gizmo_.Manipulate(
                        renderer_->GetViewMatrix(),
                        renderer_->GetProjectionMatrix(),
                        *modelTransformations_[activeModel_]
                );
                if (ImGuizmo::IsUsing()) { // Also check IsUsing for continuous drag
                    EnforceGridConstraint(activeModel_);
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
        openModelPropertiesDialog();
        ImGui::Render();

        int fb_w, fb_h;
        glfwGetFramebufferSize(window_, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);
        ImVec4 clearCol = ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg];
        glClearColor(clearCol.x * clearCol.w, clearCol.y * clearCol.w, clearCol.z * clearCol.w, clearCol.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backupCtx = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCtx);
        }
        glfwSwapBuffers(window_);
    }
}

void Application::Cleanup() { /* ... as before ... */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}