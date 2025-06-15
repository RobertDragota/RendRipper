#include "Application.h"
#include <iostream>

/**
 * @brief Construct a new Application object.
 *
 * Initializes the window, renderer and UI components.
 *
 * @param w Width of the window.
 * @param h Height of the window.
 * @param title Title of the window.
 */
Application::Application(int w, int h, const char* title)
    : width_(w), height_(h),
      window_(std::make_unique<WindowManager>(width_, height_, title)),
      renderer_(std::make_unique<SceneRenderer>(A1MINI_PRINTER_SETTINGS_FILE)),
      ui_(modelManager_, renderer_.get(), gizmo_, camera_, window_->GetWindow()) {
    if (renderer_) {
        renderer_->SetViewportSize(width_, height_);
    }
}

/**
 * @brief Destroy the Application object and perform cleanup.
 */
Application::~Application() { Cleanup(); }

/**
 * @brief Execute the application main loop until the window is closed.
 */
void Application::Run() {
    while (!window_->ShouldClose()) {
        window_->PollEvents();
        window_->BeginFrame();
        ui_.Frame();
        window_->EndFrame();
    }
}

/**
 * @brief Release owned resources.
 */
void Application::Cleanup() {
    window_.reset();
}
