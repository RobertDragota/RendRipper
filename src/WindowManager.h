#pragma once
#include <glad/glad.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>

class WindowManager {
public:
    WindowManager(int width, int height, const char* title);
    ~WindowManager();

    void PollEvents();
    bool ShouldClose() const;
    void BeginFrame();
    void EndFrame();
    GLFWwindow* GetWindow() const { return window_; }

private:
    void InitGLFW();
    void InitWindow(const char* title, int w, int h);
    void InitGLAD();
    void InitImGui();
    void Cleanup();
    GLFWwindow* window_{};
};
