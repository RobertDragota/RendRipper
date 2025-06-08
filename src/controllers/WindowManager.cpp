#include "WindowManager.h"
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <vector>
#include <imgui.h>

static void ApplyModernDarkStyle() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;
    ImVec4 bg_main_very_dark = ImVec4(0.02f, 0.02f, 0.05f, 1.00f);
    ImVec4 bg_main = ImVec4(0.05f, 0.05f, 0.08f, 1.00f);
    ImVec4 bg_secondary = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    ImVec4 bg_widget = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    ImVec4 text_main = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    ImVec4 text_disabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    ImVec4 accent_main = ImVec4(0.65f, 0.20f, 0.80f, 1.00f);
    ImVec4 accent_hover = ImVec4(0.75f, 0.30f, 0.90f, 1.00f);
    ImVec4 accent_active = ImVec4(0.55f, 0.10f, 0.70f, 1.00f);
    ImVec4 border_main = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    ImVec4 border_light = ImVec4(0.27f, 0.27f, 0.32f, 1.00f);
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
    colors[ImGuiCol_SeparatorHovered] = ImVec4(border_light.x * 1.2f, border_light.y * 1.2f, border_light.z * 1.2f, 1.0f);
    colors[ImGuiCol_SeparatorActive] = accent_main;
    colors[ImGuiCol_ResizeGrip] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.78f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(accent_main.x, accent_main.y, accent_main.z, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(bg_secondary.x * 0.9f, bg_secondary.y * 0.9f, bg_secondary.z * 0.9f, 0.9f);
    colors[ImGuiCol_TabHovered] = accent_hover;
    colors[ImGuiCol_TabActive] = accent_main;
    colors[ImGuiCol_TabUnfocused] = ImVec4(bg_secondary.x * 0.8f, bg_secondary.y * 0.8f, bg_secondary.z * 0.8f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(accent_main.x * 0.8f, accent_main.y * 0.8f, accent_main.z * 0.8f, 1.00f);
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

static void glfw_error_callback(int error, const char* desc) {
    std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
}

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

WindowManager::WindowManager(int width, int height, const char* title) {
    InitGLFW();
    InitWindow(title, width, height);
    InitGLAD();
    InitImGui();
}

WindowManager::~WindowManager() { Cleanup(); }

void WindowManager::InitGLFW() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to init GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void WindowManager::InitWindow(const char* title, int w, int h) {
    window_ = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("No GLFW window");
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
}

void WindowManager::InitGLAD() {
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("No GLAD");
    }
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
}

void WindowManager::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.Fonts->AddFontDefault();
    const char* fontRel = "../assets/fonts/Orbitron-Bold.ttf";

    ImFont* orbitron = nullptr;

    orbitron = io.Fonts->AddFontFromFileTTF(fontRel, 16.0f);

    if (orbitron) io.FontDefault = orbitron;
    ApplyModernDarkStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void WindowManager::PollEvents() { glfwPollEvents(); }

bool WindowManager::ShouldClose() const { return glfwWindowShouldClose(window_); }

void WindowManager::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void WindowManager::EndFrame() {
    ImGui::Render();
    int fb_w, fb_h;
    glfwGetFramebufferSize(window_, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);
    ImVec4 clearCol = ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg];
    glClearColor(clearCol.x * clearCol.w, clearCol.y * clearCol.w, clearCol.z * clearCol.w, clearCol.w);
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

void WindowManager::Cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}
