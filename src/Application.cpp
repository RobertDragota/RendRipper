#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <cmath>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "SceneRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "Transform.h"
#include "GizmoController.h"

#ifdef _WIN32

#include <windows.h>
#include <thread>
#include <future>
#include <regex>

#endif

struct WorldMinMaxZ
{
    float minZ;
    float maxZ;
};

WorldMinMaxZ CalculateWorldMinMaxZ(const Model &model, const Transform &transform)
{
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
    for (auto localCorner: localCorners)
        {
        glm::vec3 worldCorner = glm::vec3(modelMatrix * glm::vec4(localCorner, 1.0f));
        result.minZ = glm::min(result.minZ, worldCorner.z);
        result.maxZ = glm::max(result.maxZ, worldCorner.z);
        }
    return result;
}

void ApplyModernDarkStyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;
    ImVec4 bg_main_very_dark = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    ImVec4 bg_main = ImVec4(0.118f, 0.118f, 0.118f, 1.00f);
    ImVec4 bg_secondary = ImVec4(0.145f, 0.145f, 0.149f, 1.00f);
    ImVec4 bg_widget = ImVec4(0.200f, 0.200f, 0.200f, 1.00f);
    ImVec4 text_main = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    ImVec4 text_disabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    ImVec4 accent_main = ImVec4(0.000f, 0.478f, 0.800f, 1.00f);
    ImVec4 accent_hover = ImVec4(0.100f, 0.578f, 0.900f, 1.00f);
    ImVec4 accent_active = ImVec4(0.000f, 0.361f, 0.600f, 1.00f);
    ImVec4 border_main = ImVec4(0.220f, 0.220f, 0.220f, 1.00f);
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

static void glfw_error_callback(int error, const char *desc)
{
    std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
}

static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    glViewport(0, 0, w, h);
}

Application::Application(int w, int h, const char *title)
    : width_(w), height_(h), activeModel_(-1), showWinDialog(false)
{
    InitGLFW();
    InitWindow(title);
    InitGLAD();
    InitImGui();
    try
        {
        renderer_ = std::make_unique<SceneRenderer>(
            R"(D:\Licenta\RendRipper\Slicer\CuraEngine\build\Release\bambulab_a1mini.def.json)"); {
            float halfX = renderer_->GetBedHalfWidth(); // e.g.  90.0f
            float halfY = renderer_->GetBedHalfDepth(); // e.g.  90.0f
            // No rotation, no scale—only translate the gizmo to the bed’s reference corner
            bedCornerGizmoTransform_.translation = glm::vec3(-halfX, -halfY, 0.0f);
            bedCornerGizmoTransform_.rotationQuat = glm::quat(1.0f, 0, 0, 0);
            bedCornerGizmoTransform_.scale = glm::vec3(1.0f);
        }
        if (renderer_) { renderer_->SetViewportSize(width_, height_); }
        }
    catch (const std::exception &e)
        {
        std::cerr << "Failed to init SceneRenderer: " << e.what() << std::endl;
        throw;
        }
}

Application::~Application() { Cleanup(); }

void Application::Run() { MainLoop(); }

void Application::InitGLFW()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        throw std::runtime_error("Failed to init GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void Application::InitWindow(const char *title)
{
    window_ = glfwCreateWindow(width_, height_, title, nullptr, nullptr);
    if (!window_)
        {
        glfwTerminate();
        throw std::runtime_error("No GLFW window");
        }
    glfwSetWindowUserPointer(window_, this);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    glfwSetCursorPosCallback(window_, [](GLFWwindow *w, double x, double y)
                                 {
                                 auto *app = static_cast<Application *>(glfwGetWindowUserPointer(w));
                                 if (!app)
                                     return;
                                 static double lx = x, ly = y;
                                 static bool fm = true;
                                 if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                                     {
                                     if (ImGui::GetIO().WantCaptureMouse)
                                         {
                                         fm = true;
                                         return;
                                         }
                                     if (fm)
                                         {
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
                                     }
                                 else
                                     fm = true;
                                 });
    glfwSetScrollCallback(window_, [](GLFWwindow *w, double, double yo)
                              {
                              auto *app = static_cast<Application *>(glfwGetWindowUserPointer(w));
                              if (!app || ImGui::GetIO().WantCaptureMouse)
                                  return;
                              float zs = 0.8f;
                              app->cameraDistance_ -= static_cast<float>(yo) * zs;
                              app->cameraDistance_ = glm::clamp(app->cameraDistance_, 2.f, 150.f);
                              });
}

void Application::InitGLAD() const
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
        if (window_)
            glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("No GLAD");
        }
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
}

void Application::InitImGui() const
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ApplyModernDarkStyle();
    ImGuiStyle &s = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
        s.WindowRounding = 0;
        s.Colors[ImGuiCol_WindowBg].w = 1;
        }
    std::string fp = "../../assets/fonts/Orbitron-Bold.ttf";
    if (!io.Fonts->AddFontFromFileTTF(fp.c_str(), 16.f))
        std::cerr << "Warn:FontFail " << fp << std::endl;
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

static bool RayIntersectSphere
(
    const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 center, float radius,
    float &tOut
)
{
    glm::dvec3 O = glm::dvec3(orig) - glm::dvec3(center);
    glm::dvec3 D = glm::dvec3(dir);
    double a = glm::dot(D, D), b = 2. * glm::dot(D, O), c = glm::dot(O, O) - (double) radius * radius;
    double disc = b * b - 4. * a * c;
    if (disc < 0.)
        return false;
    double sD = std::sqrt(disc);
    double q = -0.5 * (b + (b > 0. ? sD : -sD));
    double t0 = q / a, t1 = c / q;
    if (t0 > t1)
        std::swap(t0, t1);
    if (t1 < 0.)
        return false;
    tOut = (t0 < 0.) ? static_cast<float>(t1) : static_cast<float>(t0);
    return true;
}

void Application::cameraView(glm::mat4 &view, glm::vec3 &cameraWorldPosition) const
{
    /* ... as before (Z-up) ... */
    float rP = glm::radians(cameraPitch_), rY = glm::radians(cameraYaw_);
    cameraWorldPosition.x = cameraDistance_ * cos(rP) * cos(rY);
    cameraWorldPosition.y = cameraDistance_ * cos(rP) * sin(rY);
    cameraWorldPosition.z = cameraDistance_ * sin(rP);
    glm::vec3 piv(0.f), up(0.f, 0.f, 1.f);
    view = glm::lookAt(cameraWorldPosition + piv, piv, up);
}

void Application::showMenuBar()
{
    if (ImGui::BeginMainMenuBar())
        {
        if (ImGui::BeginMenu("File"))
            {
            if (ImGui::MenuItem("Open 3D Model..."))
                {
                openFileDialog([this](std::string &selectedPath)
                    {

                    this->loadModel(selectedPath);
                    });

                }
            if (ImGui::MenuItem("Open G-code...")) {
                openFileDialog([this](std::string &selectedPath){
                    // 1) Create & parse the G-code file
                    try {
                        gcodeModel_ = std::make_shared<GCodeModel>(selectedPath);
                        // 2) Give it to the SceneRenderer
                        renderer_->SetGCodeModel(gcodeModel_);
                        // 3) Reset the current layer index to “all” (draw everything)
                        currentGCodeLayer_ = -1;
                        // You might also want to adjust the camera so you can see the whole print volume here.
                    } catch (const std::exception &e) {
                        std::cerr << "Failed to load G-code: " << e.what() << std::endl;
                    }
                });
            }
            if (activeModel_ != -1 && ImGui::MenuItem("Slice Model")) {
                sliceActiveModel();
            }
            if (ImGui::MenuItem("Exit"))
                {
                glfwSetWindowShouldClose(window_, true);
                }

            ImGui::EndMenu();
            }

        if (ImGui::BeginMenu("Generate 3D Model"))
            {

            if (ImGui::MenuItem("From Image"))
                {
                openFileDialog([this](std::string &selectedPath)
                    {
                    this->loadImageFor3DModel(selectedPath);
                    });
                }
            ImGui::EndMenu();
            }

        if (ImGui::BeginMenu("Edit"))
            {
            if (activeModel_ != -1)
                {
                if (ImGui::MenuItem("Translate", "T", gizmo_.GetCurrentMode() == ImGuizmo::TRANSLATE))
                    gizmo_.SetCurrentMode(ImGuizmo::TRANSLATE);
                if (ImGui::MenuItem("Rotate", "R", gizmo_.GetCurrentMode() == ImGuizmo::ROTATE))
                    gizmo_.SetCurrentMode(ImGuizmo::ROTATE);
                if (ImGui::MenuItem("Scale", "S", gizmo_.GetCurrentMode() == ImGuizmo::SCALE))
                    gizmo_.SetCurrentMode(ImGuizmo::SCALE);
                }
            else
                ImGui::TextDisabled("No model selected");
            ImGui::EndMenu();
            }

        ImGui::EndMainMenuBar();
        }
}

void Application::EnforceGridConstraint(int modelIndex)
{
    if (modelIndex < 0 || modelIndex >= static_cast<int>(models_.size()))
        return;

    Model &model = *models_[modelIndex];
    Transform &transform = *modelTransformations_[modelIndex];

    WorldMinMaxZ worldZ = CalculateWorldMinMaxZ(model, transform);
    float groundPlaneZ = 0.0f;

    if (worldZ.minZ < groundPlaneZ)
        {
        float offsetNeeded = groundPlaneZ - worldZ.minZ;
        transform.translation.z += offsetNeeded;
        }
}

void Application::openFileDialog(const std::function<void(std::string &)> &onFileSelected)
{
#ifdef _WIN32
    OPENFILENAMEA ofn{};
    char szFile[MAX_PATH] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = glfwGetWin32Window(window_);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "OBJ Files\0*.obj;*.OBJ\0STL Files\0*.stl;*.STL\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == TRUE)
        {
        std::string filePath(szFile);
        onFileSelected(filePath);
        }
#else
    std::cerr<<"File dialog N/A\n";
#endif

}

void Application::loadImageFor3DModel(std::string &imagePath)
{

    generating_.store(true, std::memory_order_release);
    generationDone_.store(false, std::memory_order_release);
    progress_.store(0.0f, std::memory_order_release);

    const std::vector<std::string> stages = {
            "Initializing model",
            "Processing images",
            "Running model",
            "Extracting mesh",
            "Exporting mesh"
            };
    const int numStages = static_cast<int>(stages.size());

    std::thread([this, imagePath, stages, numStages]()
        {

        {
            std::lock_guard lk(generationMessageMutex_);
            generationMessage_ = "Launching TripoSR";
        }
        progress_.store(0.0f, std::memory_order_release);

        std::string cmd =
                std::string(PYTHON_EXECUTABLE) +
                " -u " +
                std::string(GENERATE_MODEL_SCRIPT) +
                " \"" + imagePath + "\"" +
                " --chunk-size 8192" +
                " --device cuda:0" +
                " --mc-resolution 256" +
                " --output-dir " +
                std::string(OUTPUT_DIR) +
                " 2>&1";

#ifdef _WIN32
        FILE *pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe)
            {
            std::lock_guard lk(generationMessageMutex_);
            generationMessage_ = "Failed to start TripoSR process.";
            generationDone_.store(true, std::memory_order_release);
            progress_.store(1.0f, std::memory_order_release);
            return;
            }

        char buffer[512];

        while (fgets(buffer, sizeof(buffer), pipe))
            {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n')
                line.pop_back(); {
                std::lock_guard lk(generationMessageMutex_);
                generationMessage_ = line;
            }

            for (int i = 0; i < numStages; ++i)
                {
                const auto &stage = stages[i];
                if (line.find(stage + " ...") != std::string::npos)
                    {

                    float f = static_cast<float>(i) / static_cast<float>(numStages);
                    progress_.store(f, std::memory_order_release);
                    break;
                    }
                if (line.find(stage + " finished") != std::string::npos)
                    {

                    float f = static_cast<float>(i + 1) / static_cast<float>(numStages);
                    progress_.store(f, std::memory_order_release);
                    break;
                    }
                }
            }

#ifdef _WIN32
        int ret = _pclose(pipe);
#else
        int ret = pclose(pipe);
#endif

        {
            std::lock_guard lk(generationMessageMutex_);
            if (ret == 0)
                {
                generationMessage_ = "Generation complete!";
                }
            else
                {
                generationMessage_ = "Generation failed (code " + std::to_string(ret) + ")";
                }
        }
        progress_.store(1.0f, std::memory_order_release);
        generationDone_.store(true, std::memory_order_release);

        }).detach();
}

void Application::showErrorModal(std::string &message)
{
    if (showErrorModal_)
        {
        ImGui::OpenPopup("Error");

        ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(
            ImGui::GetMainViewport()->GetCenter(),
            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
        );
        }

    if (ImGui::BeginPopupModal("Error", &showErrorModal_, ImGuiWindowFlags_AlwaysAutoResize))
        {

        ImGui::Text("%s", errorModalMessage_.c_str());
        ImGui::Spacing();

        ImGui::EndPopup();
        }
}

void Application::showGenerationModal()
{
    if (generating_.load())
        {
        ImGui::OpenPopup("Generating Model");
        generating_.store(false);
        }

    ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
    );

    if (ImGui::BeginPopupModal("Generating Model", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
        {

        {
            const char *prompt = "Please wait";
            float ww = ImGui::GetWindowWidth();
            float tw = ImGui::CalcTextSize(prompt).x;
            ImGui::SetCursorPosX((ww - tw) * 0.5f);
            ImGui::Text("%s", prompt);
        }

        ImGui::Spacing(); {
            float fraction = progress_.load();
            const float radius = 120.0f;
            const float thickness = 8.0f;
            ImU32 fg = IM_COL32(75, 175, 255, 255);
            ImU32 bg = IM_COL32(60, 60, 60, 128);

            float ww = ImGui::GetWindowWidth();
            ImGui::SetCursorPosX((ww - radius * 2.0f) * 0.5f);
            ImGui::ProgressBar("##progress",
                               fraction,
                               radius,
                               thickness,
                               fg,
                               bg);
        }

        ImGui::Spacing(); {
            std::lock_guard<std::mutex> lk(generationMessageMutex_);
            std::string msg = generationMessage_;
            float ww = ImGui::GetWindowWidth();
            float tw = ImGui::CalcTextSize(msg.c_str()).x;
            ImGui::SetCursorPosX((ww - tw) * 0.5f);
            ImGui::Text("%s", msg.c_str());
        }

        if (generationDone_.load())
            {
            ImGui::Spacing();
            float ww = ImGui::GetWindowWidth();
            float bw = 120.0f;
            ImGui::SetCursorPosX((ww - bw) * 0.5f);
            if (ImGui::Button("Close", ImVec2(bw, 0)))
                {
                ImGui::CloseCurrentPopup();
                std::string path = std::string(OUTPUT_DIR) + "/0/mesh.obj";
                loadModel(path);
                generationDone_.store(false);
                }
            }

        ImGui::EndPopup();
        }
}

void Application::showSlicingModal()
{
    if (slicing_.load())
        {
        ImGui::OpenPopup("Slicing Model");
        slicing_.store(false);
        }

    ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
    );

    if (ImGui::BeginPopupModal("Slicing Model", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
        {

        const char *prompt = "Please wait";
        float ww = ImGui::GetWindowWidth();
        float tw = ImGui::CalcTextSize(prompt).x;
        ImGui::SetCursorPosX((ww - tw) * 0.5f);
        ImGui::Text("%s", prompt);

        ImGui::Spacing();
        float fraction = slicingProgress_.load();
        const float radius = 120.0f;
        const float thickness = 8.0f;
        ImU32 fg = IM_COL32(75, 175, 255, 255);
        ImU32 bg = IM_COL32(60, 60, 60, 128);

        ImGui::SetCursorPosX((ww - radius * 2.0f) * 0.5f);
        ImGui::ProgressBar("##slice_progress",
                           fraction,
                           radius,
                           thickness,
                           fg,
                           bg);

        ImGui::Spacing();
        {
            std::lock_guard<std::mutex> lk(slicingMessageMutex_);
            std::string msg = slicingMessage_;
            float tw2 = ImGui::CalcTextSize(msg.c_str()).x;
            ImGui::SetCursorPosX((ww - tw2) * 0.5f);
            ImGui::Text("%s", msg.c_str());
        }

        if (slicingDone_.load())
            {
            ImGui::Spacing();
            float bw = 120.0f;
            ImGui::SetCursorPosX((ww - bw) * 0.5f);
            if (ImGui::Button("Close", ImVec2(bw, 0)))
                {
                ImGui::CloseCurrentPopup();
                slicingDone_.store(false);
                }
            }

        ImGui::EndPopup();
        }
}

void Application::sliceActiveModel()
{
    if (activeModel_ < 0 || activeModel_ >= static_cast<int>(loadedModelPaths_.size()))
        return;

    std::string stlPath = loadedModelPaths_[activeModel_];
    slicing_.store(true);
    slicingDone_.store(false);
    slicingProgress_.store(0.0f);

    std::thread([this, stlPath]()
        {
            std::filesystem::path dragon = std::filesystem::path("Slicer/CuraEngine/build/Release/dragon_settings.json");
            std::filesystem::path model = std::filesystem::path("Slicer/CuraEngine/build/Release/model_settings.json");
            try
                {
                std::filesystem::copy_file(dragon, model, std::filesystem::copy_options::overwrite_existing);
                }
            catch (const std::exception &e)
                {
                std::lock_guard lk(slicingMessageMutex_);
                slicingMessage_ = std::string("Settings copy failed: ") + e.what();
                }

            std::filesystem::path output = std::filesystem::path(stlPath).replace_extension(".gcode");
            std::string cmd = std::string(CURA_ENGINE_EXE) +
                              " slice -j fdmprinter.def.json -j bambulab_base.def.json -j bambulab_a1mini.def.json -j model_settings.json" +
                              " -l \"" + stlPath + "\"" +
                              " -o \"" + output.string() + "\" 2>&1";

#ifdef _WIN32
            FILE *pipe = _popen(cmd.c_str(), "r");
#else
            FILE *pipe = popen(cmd.c_str(), "r");
#endif
            if (!pipe)
                {
                std::lock_guard lk(slicingMessageMutex_);
                slicingMessage_ = "Failed to start CuraEngine.";
                slicingDone_.store(true);
                slicingProgress_.store(1.0f);
                return;
                }

            char buffer[512];
            while (fgets(buffer, sizeof(buffer), pipe))
                {
                std::string line(buffer);
                if (!line.empty() && line.back() == '\n')
                    line.pop_back();
                {
                    std::lock_guard lk(slicingMessageMutex_);
                    slicingMessage_ = line;
                }
                }
#ifdef _WIN32
            int ret = _pclose(pipe);
#else
            int ret = pclose(pipe);
#endif
            {
                std::lock_guard lk(slicingMessageMutex_);
                slicingMessage_ = (ret == 0) ? "Slicing complete!" : ("Slicing failed (code " + std::to_string(ret) + ")");
            }
            slicingProgress_.store(1.0f);
            slicingDone_.store(true);

            if (ret == 0)
                {
                try
                    {
                    auto gm = std::make_shared<GCodeModel>(output.string());
                    renderer_->SetGCodeModel(gm);
                    gcodeModel_ = gm;
                    currentGCodeLayer_ = -1;
                    }
                catch (const std::exception &e)
                    {
                    std::lock_guard lk(slicingMessageMutex_);
                    slicingMessage_ += std::string(" | Load failed: ") + e.what();
                    }
                }
        }).detach();
}

void Application::loadModel(std::string &modelPath)
{
    try
        {
        std::string directory = modelPath.substr(0, modelPath.find_last_of("/\\"));
        std::string output = directory + "/output.stl";
        MeshRepairer::repairSTLFile(modelPath, output);
        auto mPtr = std::make_unique<Model>(output);
        mPtr->computeBounds();

        glm::vec3 minB = mPtr->minBounds; // e.g. (–25, –10, 0) mm
        glm::vec3 maxB = mPtr->maxBounds; // e.g. (+25, +15, 40) mm
        glm::vec3 size = maxB - minB;

        loadedMeshDimensions_.push_back(size);
        loadedModelPaths_.push_back(output);

        glm::vec3 oC = mPtr->center, oMin = mPtr->minBounds, oMax = mPtr->maxBounds;
        auto t = std::make_unique<Transform>();

        glm::vec3 corners[8] = {
                {oMin.x, oMin.y, oMin.z},
                {oMax.x, oMin.y, oMin.z},
                {oMin.x, oMax.y, oMin.z},
                {oMax.x, oMax.y, oMin.z},
                {oMin.x, oMin.y, oMax.z},
                {oMax.x, oMin.y, oMax.z},
                {oMin.x, oMax.y, oMax.z},
                {oMax.x, oMax.y, oMax.z}
                };
        float minZ = corners[0].z;
        for (int i = 1; i < 8; ++i)
            minZ = std::min(minZ, corners[i].z);

        // Center in X/Y:
        glm::vec3 c = oC; // center in mm
        t->translation = glm::vec3(-c.x, -c.y, -minZ);
        t->scale = glm::vec3(1.0f); // no scaling: 1 unit = 1 mm
        t->rotationQuat = glm::quat(1, 0, 0, 0);

        modelTransformations_.push_back(std::move(t));
        modelShaders_.emplace_back(std::make_unique<Shader>("../../resources/shaders/model_shader.vert",
                                                            "../../resources/shaders/model_shader.frag"));
        models_.emplace_back(std::move(mPtr));
        activeModel_ = static_cast<int>(models_.size() - 1);
        EnforceGridConstraint(activeModel_);
        }
    catch (const std::exception &e)
        {
        errorModalMessage_ = "Failed to load model: " + modelPath + "\n" + e.what();
        std::cout << errorModalMessage_ << std::endl;
        showErrorModal_ = true;
        }

}

void Application::UnloadModel(int idx)
{
    if (idx < 0 || idx >= models_.size())
        return;
    models_.erase(models_.begin() + idx);
    modelShaders_.erase(modelShaders_.begin() + idx);
    modelTransformations_.erase(modelTransformations_.begin() + idx);
    loadedMeshDimensions_.erase(loadedMeshDimensions_.begin() + idx);
    loadedModelPaths_.erase(loadedModelPaths_.begin() + idx);
    if (activeModel_ == idx)
        activeModel_ = -1;
    else if (activeModel_ > idx)
        activeModel_--;
}

void Application::openRenderScene()
{
    ImGuiViewport *vp = ImGui::GetMainViewport();
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

void Application::openModelPropertiesDialog()
{
    ImGui::Begin("Model Properties");

    // If there is a valid “activeModel_” index, show its transform:
    if (activeModel_ >= 0 && activeModel_ < static_cast<int>(modelTransformations_.size()))
        {
        ImGui::Text("Model Index: %d", activeModel_);

        glm::vec3 dims = loadedMeshDimensions_[activeModel_];
        ImGui::Text("Real Dimensions (mm): %.2f × %.2f × %.2f",
                    dims.x, dims.y, dims.z);

        ImGui::Separator();

        // Reference to the active Transform:
        auto &tf = *modelTransformations_[activeModel_];
        bool changed = false;

        //
        // Build a 4‐column table: [ (empty) | "X" | "Y" | "Z" ]
        // Column 0 will hold the row label (Translation/Rotation/Scale).
        // Columns 1,2,3 hold the DragFloat for X, Y, Z respectively.
        //
        if (ImGui::BeginTable("TransformTable", 4,
                              ImGuiTableFlags_Resizable // allow user to drag‐resize columns
                              | ImGuiTableFlags_NoSavedSettings // do not persist column widths
        ))
            {
            // ─── Setup Columns ──────────────────────────────────────────────────────────
            // Column 0: a fixed width (enough to hold “Translation” etc.)
            ImGui::TableSetupColumn(
                "", // no visible header text for column 0
                ImGuiTableColumnFlags_WidthFixed, // fixed width
                100.0f // 100 px is enough for “Translation”
            );
            // Column 1: “X” header, stretchable
            ImGui::TableSetupColumn("X",
                                    ImGuiTableColumnFlags_WidthStretch, // fill remaining space proportionally
                                    0.0f // weight = 0 means “auto distribute”
            );
            // Column 2: “Y” header, stretchable
            ImGui::TableSetupColumn("Y",
                                    ImGuiTableColumnFlags_WidthStretch,
                                    0.0f
            );
            // Column 3: “Z” header, stretchable
            ImGui::TableSetupColumn("Z",
                                    ImGuiTableColumnFlags_WidthStretch,
                                    0.0f
            );

            // Draw the header row (will show “X”, “Y”, “Z” above columns 1–3)
            ImGui::TableHeadersRow();
            //───────────────────────────────────────────────────────────────────────────────

            //
            // Row 1: Translation
            //
            ImGui::TableNextRow();
            // Column 0: “Translation” label
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Translation");

            // Column 1: DragFloat for tf.translation.x
            ImGui::TableSetColumnIndex(1);
            // This SetNextItemWidth(-FLT_MIN) makes the DragFloat fill the entire column
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##TransX", &tf.translation.x, 0.01f))
                {
                changed = true;
                }

            // Column 2: DragFloat for tf.translation.y
            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##TransY", &tf.translation.y, 0.01f))
                {
                changed = true;
                }

            // Column 3: DragFloat for tf.translation.z
            ImGui::TableSetColumnIndex(3);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##TransZ", &tf.translation.z, 0.01f))
                {
                changed = true;
                }

            //
            // Row 2: Rotation (Euler angles)
            // We will read them into a temporary glm::vec3, then write back
            //
            glm::vec3 euler = tf.getEulerAngles();

            ImGui::TableNextRow();
            // Column 0: “Rotation” label
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Rotation");

            // Column 1: DragFloat for euler.x
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##RotX", &euler.x, 0.5f))
                {
                tf.setEulerAngles(euler);
                changed = true;
                }

            // Column 2: DragFloat for euler.y
            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##RotY", &euler.y, 0.5f))
                {
                tf.setEulerAngles(euler);
                changed = true;
                }

            // Column 3: DragFloat for euler.z
            ImGui::TableSetColumnIndex(3);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##RotZ", &euler.z, 0.5f))
                {
                tf.setEulerAngles(euler);
                changed = true;
                }

            //
            // Row 3: Scale
            //
            ImGui::TableNextRow();
            // Column 0: “Scale” label
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Scale");

            // Column 1: DragFloat for tf.scale.x
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##ScaleX", &tf.scale.x, 0.01f, 0.001f, 100.0f))
                {
                changed = true;
                }

            // Column 2: DragFloat for tf.scale.y
            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##ScaleY", &tf.scale.y, 0.01f, 0.001f, 100.0f))
                {
                changed = true;
                }

            // Column 3: DragFloat for tf.scale.z
            ImGui::TableSetColumnIndex(3);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::DragFloat("##ScaleZ", &tf.scale.z, 0.01f, 0.001f, 100.0f))
                {
                changed = true;
                }

            ImGui::EndTable();
            }

        // If any transform changed, enforce the bed‐constraint:
        if (changed)
            {
            EnforceGridConstraint(activeModel_);
            }

        ImGui::Separator();

        // Reset/Unload buttons remain the same:
        if (ImGui::Button("Reset Transform"))
            {
            // … (your existing Reset Transform code) …
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
            for (int i = 1; i < 8; ++i)
                {
                newMin = glm::min(newMin, c[i]);
                }
            glm::vec3 centerRot = tf.rotationQuat * oC;
            tf.translation = glm::vec3(-centerRot.x, -centerRot.y, -newMin.z);
            tf.scale = glm::vec3(1.0f);
            EnforceGridConstraint(activeModel_);
            }
        ImGui::SameLine();
        if (ImGui::Button("Unload Model"))
            {
            UnloadModel(activeModel_);
            }

        }
    else
        {
        ImGui::Text("No model selected.");
        }


    if (gcodeModel_) {
        int layerCount = gcodeModel_->GetLayerCount();
        auto layerHeights = gcodeModel_->GetLayerHeights();

        // Keep currentGCodeLayer_ in [-1..layerCount-1]
        if (currentGCodeLayer_ < -1) currentGCodeLayer_ = -1;
        if (currentGCodeLayer_ > layerCount - 1) currentGCodeLayer_ = layerCount - 1;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // small indent
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        ImGui::BeginGroup();
        ImGui::Text("G-code Layers:");
        // Show a slider from 0..layerCount-1, or -1 = “All layers”
        if (layerCount > 0) {
            // Build a tooltip string showing Z for each layer value:
            ImGui::Text("Z = %.2f mm (layer %d of %d)",
                        layerHeights[ (currentGCodeLayer_ < 0 ? 0 : currentGCodeLayer_) ],
                        (currentGCodeLayer_ < 0 ? 0 : currentGCodeLayer_), layerCount - 1);

            // Slider from -1..(layerCount-1)
            ImGui::SliderInt("Layer", &currentGCodeLayer_, -1, layerCount - 1,
                             currentGCodeLayer_ < 0 ? "All" : "%d");
        } else {
            ImGui::Text("No layers found in G-code");
        }
        ImGui::EndGroup();
    }

    ImGui::End();
}

void Application::getActiveModel
(
    glm::mat4 &viewMatrix, const ImVec2 &viewportScreenPos,
    const ImVec2 &viewportSize
)
{
    float nearT = std::numeric_limits<float>::infinity();
    int pick = -1;
    ImVec2 mG = ImGui::GetMousePos();
    float rX = mG.x - viewportScreenPos.x, rY = mG.y - viewportScreenPos.y;
    float nX = (2.f * rX) / viewportSize.x - 1.f, nY = 1.f - (2.f * rY) / viewportSize.y;
    if (!renderer_ || viewportSize.x <= 0 || viewportSize.y <= 0)
        return;
    glm::mat4 iP = glm::inverse(renderer_->GetProjectionMatrix());
    glm::vec4 rE = iP * glm::vec4(nX, nY, -1, 1);
    rE = glm::vec4(rE.x, rE.y, -1, 0);
    glm::mat4 iV = glm::inverse(viewMatrix);
    glm::vec3 rO = glm::vec3(iV[3]), rD = glm::normalize(glm::vec3(iV * rE));
    for (size_t i = 0; i < models_.size(); ++i)
        {
        if (!models_[i] || !modelTransformations_[i])
            continue;
        float t;
        glm::mat4 mW = modelTransformations_[i]->getMatrix();
        glm::vec3 wC = glm::vec3(mW * glm::vec4(models_[i]->center, 1));
        float mS = glm::max(modelTransformations_[i]->scale.x,
                            glm::max(modelTransformations_[i]->scale.y, modelTransformations_[i]->scale.z));
        float wR = models_[i]->radius * mS;
        if (RayIntersectSphere(rO, rD, wC, wR, t) && t < nearT)
            {
            nearT = t;
            pick = static_cast<int>(i);
            }
        }
    activeModel_ = pick;
}

void Application::renderModels(glm::mat4 &)
{
    for (size_t i = 0; i < models_.size(); ++i)
        {
        if (models_[i] && modelShaders_[i] && modelTransformations_[i] && renderer_ && modelShaders_[i]->ID != 0)
            {
            renderer_->RenderModel(*models_[i], *modelShaders_[i], *modelTransformations_[i]);
            }
        }
}



void Application::MainLoop()
{
    while (!glfwWindowShouldClose(window_))
        {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        glm::mat4 viewMat;
        glm::vec3 camWorldPos;
        cameraView(viewMat, camWorldPos);
        showMenuBar();
        openRenderScene();
        showGenerationModal();
        showSlicingModal();
        showErrorModal(errorModalMessage_);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("3D Viewport"); {
            //ImVec2 viewportContentStart = ImGui::GetCursorScreenPos();
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            if (viewportSize.x < 1.0f)
                viewportSize.x = 1.0f;
            if (viewportSize.y < 1.0f)
                viewportSize.y = 1.0f;

            if (renderer_)
                {
                renderer_->SetViewportSize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
                //renderer_->currentGCodeLayerIndex_ = this->currentGCodeLayer_;
                renderer_->BeginScene(viewMat, camWorldPos);
                renderModels(viewMat);
                renderer_->RenderGCodeUpToLayer(this->currentGCodeLayer_);
                renderer_->EndScene();
                GLuint texID = renderer_->GetSceneTexture();
                ImGui::Image(texID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
                }

            ImVec2 actualViewportTopLeft = ImGui::GetItemRectMin();
            ImVec2 vpTopLeft = ImGui::GetItemRectMin();
            if (ImGui::IsWindowHovered())
                {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
                    {
                    getActiveModel(viewMat, actualViewportTopLeft, viewportSize);
                    }

                // ─────────── Right‐click: orbit camera ───────────
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGuizmo::IsUsing())
                    {
                    ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
                    float sensitivity = 0.2f;
                    cameraYaw_ += delta.x * sensitivity;
                    cameraPitch_ += delta.y * sensitivity;
                    cameraPitch_ = glm::clamp(cameraPitch_, -89.0f, 89.0f);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
                    }

                // ─────────── Scroll: zoom camera ───────────
                ImGuiIO &io = ImGui::GetIO();
                if (io.MouseWheel != 0.0f && !ImGuizmo::IsUsing())
                    {
                    float zoomSpeed = 5.0f;
                    cameraDistance_ -= io.MouseWheel * zoomSpeed;
                    cameraDistance_ = glm::clamp(cameraDistance_, 2.0f, 800.0f);
                    }
                }

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(actualViewportTopLeft.x, actualViewportTopLeft.y, viewportSize.x, viewportSize.y);
            if (activeModel_ >= 0 && activeModel_ < static_cast<int>(models_.size()) && renderer_)
                {
                gizmo_.Manipulate(
                    renderer_->GetViewMatrix(),
                    renderer_->GetProjectionMatrix(),
                    *modelTransformations_[activeModel_]
                );
                if (ImGuizmo::IsUsing())
                    {
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
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
            GLFWwindow *backupCtx = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCtx);
            }
        glfwSwapBuffers(window_);
        }
}

void Application::Cleanup()
{

    ImGui_ImplOpenGL3_Shutdown();

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    if (window_)
        {
        glfwDestroyWindow(window_);
        window_ = nullptr;
        }

    glfwTerminate();

}
