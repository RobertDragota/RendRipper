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

Application::~Application() {
    running_.store(false);
    if (renderThread_.joinable())
        renderThread_.join();
    Cleanup();
}

void Application::Run() {
    running_.store(true);
    renderThread_ = std::thread(&Application::RenderLoop, this);
    renderThread_.join();
}

void Application::RenderLoop() {
    while (running_.load() && !window_->ShouldClose()) {
        window_->PollEvents();
        std::lock_guard<std::mutex> lk(renderMutex_);
        window_->BeginFrame();
        ui_.Frame();
        window_->EndFrame();
    }
    running_.store(false);
}

void Application::Cleanup() {
    window_.reset();
}
