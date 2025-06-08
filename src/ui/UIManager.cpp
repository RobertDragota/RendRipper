#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <glad/glad.h> // glad must be included before Windows headers
#include <windows.h>
#include <commdlg.h>
#endif

#include "UIManager.h"
#ifdef _WIN32
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#endif
#include "MeshRepairer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <functional>
#include <limits>
#include <thread>
#include <regex>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

#include "glm/gtx/intersect.hpp"

using json = nlohmann::json;



UIManager::UIManager(ModelManager& mm, SceneRenderer* renderer,
                     GizmoController& gizmo, CameraController& camera,
                     GLFWwindow* window)
    : modelManager_(mm), renderer_(renderer), gizmo_(gizmo), camera_(camera), window_(window)
{
    loadModelSettings();
    if (renderer_)
        gizmo_.SetOffset(glm::vec3(renderer_->GetBedHalfWidth() + renderer_->GetPlatformOffset().x,
                                    renderer_->GetPlatformOffset().y,
                                    renderer_->GetBedHalfDepth() + renderer_->GetPlatformOffset().z));
}

void UIManager::Frame() {
    ImGuizmo::BeginFrame();
    glm::mat4 viewMat;
    glm::vec3 camWorldPos;
    viewMat = camera_.GetViewMatrix(camWorldPos);

    showMenuBar();
    openRenderScene();
    showGenerationModal();
    showSlicingModal();
    showErrorModal(errorModalMessage_);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("3D Viewport");
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x < 1.0f) viewportSize.x = 1.0f;
    if (viewportSize.y < 1.0f) viewportSize.y = 1.0f;
    if (renderer_) {
        renderer_->SetViewportSize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
        renderer_->BeginScene(viewMat, camWorldPos);
        renderModels(viewMat);
        renderer_->RenderGCodeUpToLayer(currentGCodeLayer_);
        renderer_->EndScene();
        GLuint texID = renderer_->GetSceneTexture();
        ImGui::Image(texID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImVec2 actualViewportTopLeft = ImGui::GetItemRectMin();
    if (ImGui::IsWindowHovered()) {
        handleViewportInput(viewMat, actualViewportTopLeft, viewportSize);
    }
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(actualViewportTopLeft.x, actualViewportTopLeft.y, viewportSize.x, viewportSize.y);
    if (activeModel_ >= 0 && activeModel_ < static_cast<int>(modelManager_.Count()) && renderer_) {
        gizmo_.Manipulate(
            renderer_->GetViewMatrix(),
            renderer_->GetProjectionMatrix(),
            *modelManager_.GetTransform(activeModel_)
        );
        if (ImGuizmo::IsUsing()) {
            modelManager_.EnforceGridConstraint(activeModel_);
            modelManager_.UpdateDimensions(activeModel_);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    openModelPropertiesDialog();
}

void UIManager::showMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open 3D Model")) {
                openFileDialog([this](std::string& selected){ loadModel(selected); });
            }
            if (ImGui::MenuItem("Open G-code")) {
                openFileDialog([this](std::string& selected){
                    try {
                        gcodeModel_ = std::make_shared<GCodeModel>(selected);
                        if (renderer_) {
                            glm::vec3 c = gcodeModel_->GetCenter();
                            glm::vec3 offset(renderer_->GetBedHalfWidth() + renderer_->GetPlatformOffset().x - c.x,
                                             renderer_->GetBedHalfDepth() + renderer_->GetPlatformOffset().z - c.y,
                                             0.f);
                            renderer_->SetGCodeOffset(offset);
                            renderer_->SetGCodeModel(gcodeModel_);
                        }
                        currentGCodeLayer_ = -1;
                    } catch (const std::exception& e) {
                        std::cerr << "Failed to load G-code: " << e.what() << std::endl;
                    }
                });
            }
            if (activeModel_ != -1 && ImGui::MenuItem("Slice Model")) {
                sliceActiveModel();
            }
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Generate 3D Model")) {
            if (ImGui::MenuItem("From Image")) {
                openFileDialog([this](std::string& p){ loadImageFor3DModel(p); });
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (activeModel_ != -1) {
                if (ImGui::MenuItem("Translate", "T", gizmo_.GetCurrentMode()==ImGuizmo::TRANSLATE))
                    gizmo_.SetCurrentMode(ImGuizmo::TRANSLATE);
                if (ImGui::MenuItem("Rotate", "R", gizmo_.GetCurrentMode()==ImGuizmo::ROTATE))
                    gizmo_.SetCurrentMode(ImGuizmo::ROTATE);
                if (ImGui::MenuItem("Scale", "S", gizmo_.GetCurrentMode()==ImGuizmo::SCALE))
                    gizmo_.SetCurrentMode(ImGuizmo::SCALE);
            } else {
                ImGui::TextDisabled("No model selected");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

