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

/**
 * @brief Helper class for creating and managing the main GLFW window
 *        along with its ImGui context.
 */
class WindowManager {
public:
    /**
     * @brief Construct the window and initialize OpenGL/ImGui state.
     * @param width  Initial window width in pixels.
     * @param height Initial window height in pixels.
     * @param title  Window title string.
     */
    WindowManager(int width, int height, const char* title);

    /**
     * @brief Destroy the window and associated resources.
     */
    ~WindowManager();

    /**
     * @brief Process pending window events.
     */
    void PollEvents();

    /**
     * @brief Check if the user has requested to close the window.
     */
    bool ShouldClose() const;

    /**
     * @brief Begin a new ImGui frame and prepare for rendering.
     */
    void BeginFrame();

    /**
     * @brief Present the rendered frame and finalize ImGui rendering.
     */
    void EndFrame();

    /**
     * @brief Access the underlying GLFWwindow handle.
     */
    GLFWwindow* GetWindow() const { return window_; }

private:
    /** @brief Setup the GLFW library. */
    void InitGLFW();

    /**
     * @brief Create the GLFW window and configure callbacks.
     */
    void InitWindow(const char* title, int w, int h);

    /** @brief Load OpenGL functions using GLAD. */
    void InitGLAD();

    /** @brief Initialize the ImGui context and styling. */
    void InitImGui();

    /** @brief Cleanup and destroy window and ImGui resources. */
    void Cleanup();

    GLFWwindow* window_{};
};
