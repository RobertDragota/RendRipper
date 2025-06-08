#include "Application.h"
#include <iostream>

Application::Application(int w, int h, const char* title)
    : width_(w), height_(h),
      window_(std::make_unique<WindowManager>(width_, height_, title)),
      renderer_(std::make_unique<SceneRenderer>(A1MINI_PRINTER_SETTINGS_FILE)),
      ui_(modelManager_, renderer_.get(), gizmo_, camera_, window_->GetWindow()) {
    if (renderer_) {
        renderer_->SetViewportSize(width_, height_);
    }
}

Application::~Application() { Cleanup(); }

void Application::Run() {
    while (!window_->ShouldClose()) {
        window_->PollEvents();
        window_->BeginFrame();
        ui_.Frame();
        window_->EndFrame();
    }
}

void Application::Cleanup() {
    window_.reset();
}
