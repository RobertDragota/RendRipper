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
#include <filesystem>
#include <iostream>
#include <limits>
#include <thread>
#include <regex>
#include <cstdio>

#include "glm/gtx/intersect.hpp"

// Utility: ray-sphere intersection using GLM helpers
static bool RayIntersectSphere(const glm::vec3& origin,
                               const glm::vec3& dir,
                               const glm::vec3& center,
                               float radius,
                               float& t)
{
    // GLM requires the squared radius and a normalized direction
    return glm::intersectRaySphere(origin, glm::normalize(dir),
                                   center, radius * radius, t);
}

UIManager::UIManager(ModelManager& mm, SceneRenderer* renderer,
                     GizmoController& gizmo, CameraController& camera,
                     GLFWwindow* window)
    : modelManager_(mm), renderer_(renderer), gizmo_(gizmo), camera_(camera), window_(window) {}

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
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
            getActiveModel(viewMat, actualViewportTopLeft, viewportSize);
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGuizmo::IsUsing()) {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
            float sensitivity = 0.2f;
            camera_.yaw += delta.x * sensitivity;
            camera_.pitch += delta.y * sensitivity;
            camera_.pitch = glm::clamp(camera_.pitch, -89.0f, 89.0f);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
        }
        ImGuiIO &io = ImGui::GetIO();
        if (io.MouseWheel != 0.0f && !ImGuizmo::IsUsing()) {
            float zoomSpeed = 5.0f;
            camera_.distance -= io.MouseWheel * zoomSpeed;
            camera_.distance = glm::clamp(camera_.distance, 2.0f, 800.0f);
        }
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
            if (ImGui::MenuItem("Open 3D Model...")) {
                openFileDialog([this](std::string& selected){ loadModel(selected); });
            }
            if (ImGui::MenuItem("Open G-code...")) {
                openFileDialog([this](std::string& selected){
                    try {
                        gcodeModel_ = std::make_shared<GCodeModel>(selected);
                        if (renderer_) {
                            renderer_->SetGCodeOffset(glm::vec3(0.f));
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

void UIManager::openFileDialog(const std::function<void(std::string&)>& onFileSelected) {
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
    if (GetOpenFileNameA(&ofn) == TRUE) {
        std::string filePath(szFile);
        onFileSelected(filePath);
    }
#else
    std::cerr << "File dialog N/A\n";
#endif
}

void UIManager::loadImageFor3DModel(std::string& imagePath) {
    generating_.store(true);
    generationDone_.store(false);
    progress_.store(0.0f);
    const std::vector<std::string> stages = {
        "Initializing model",
        "Processing images",
        "Running model",
        "Extracting mesh",
        "Exporting mesh"
    };
    const int numStages = static_cast<int>(stages.size());
    std::thread([this, imagePath, stages, numStages]() {
        {
            std::lock_guard lk(generationMessageMutex_);
            generationMessage_ = "Launching TripoSR";
        }
        progress_.store(0.0f);
        std::string cmd = std::string(PYTHON_EXECUTABLE) + " -u " +
                          std::string(GENERATE_MODEL_SCRIPT) + " \"" + imagePath + "\"" +
                          " --chunk-size 8192" +
                          " --device cuda:0" +
                          " --mc-resolution 256" +
                          " --output-dir " + std::string(OUTPUT_DIR) +
                          " 2>&1";
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe) {
            std::lock_guard lk(generationMessageMutex_);
            generationMessage_ = "Failed to start TripoSR process.";
            generationDone_.store(true);
            progress_.store(1.0f);
            return;
        }
        char buffer[512];
        int currentStage = 0;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') line.pop_back();
            {
                std::lock_guard lk(generationMessageMutex_);
                generationMessage_ = line;
            }
            if (currentStage < numStages &&
                line.find(stages[currentStage]) != std::string::npos &&
                line.find("finished") != std::string::npos)
            {
                ++currentStage;
                progress_.store(static_cast<float>(currentStage) / numStages);
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
                generationMessage_ = "Generation complete!";
            else
                generationMessage_ = "Generation failed (code " + std::to_string(ret) + ")";
        }
        progress_.store(1.0f);
        generationDone_.store(true);
    }).detach();
}

void UIManager::showErrorModal(std::string& message) {
    if (showErrorModal_) {
        ImGui::OpenPopup("Error");
        ImGui::SetNextWindowSize(ImVec2(400,150), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f,0.5f));
    }
    if (ImGui::BeginPopupModal("Error", &showErrorModal_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", errorModalMessage_.c_str());
        ImGui::Spacing();
        ImGui::EndPopup();
    }
}

void UIManager::showGenerationModal() {
    if (generating_.load()) {
        ImGui::OpenPopup("Generating Model");
        generating_.store(false);
    }
    ImGui::SetNextWindowSize(ImVec2(400,350), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f,0.5f));
    if (ImGui::BeginPopupModal("Generating Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* prompt = "Please wait";
        float ww = ImGui::GetWindowWidth();
        float tw = ImGui::CalcTextSize(prompt).x;
        ImGui::SetCursorPosX((ww - tw)*0.5f);
        ImGui::Text("%s", prompt);
        ImGui::Spacing();
        float fraction = progress_.load();
        const float radius = 120.f;
        const float thickness = 8.f;
        ImU32 fg = IM_COL32(75,175,255,255);
        ImU32 bg = IM_COL32(60,60,60,128);
        ImGui::SetCursorPosX((ww - radius*2.f)*0.5f);
        ImGui::ProgressBar("##progress", fraction, radius, thickness, fg, bg);
        ImGui::Spacing();
        {
            std::lock_guard<std::mutex> lk(generationMessageMutex_);
            std::string msg = generationMessage_;
            float tw2 = ImGui::CalcTextSize(msg.c_str()).x;
            ImGui::SetCursorPosX((ww - tw2)*0.5f);
            ImGui::Text("%s", msg.c_str());
        }
        if (generationDone_.load()) {
            ImGui::Spacing();
            float bw = 120.f;
            ImGui::SetCursorPosX((ww - bw)*0.5f);
            if (ImGui::Button("Close", ImVec2(bw,0))) {
                ImGui::CloseCurrentPopup();
                std::string path = std::string(OUTPUT_DIR) + "/0/mesh.obj";
                loadModel(path);
                generationDone_.store(false);
            }
        }
        ImGui::EndPopup();
    }
}

void UIManager::showSlicingModal() {
    if (loadGcodePending_.load()) {
        finalizeSlicing();
        loadGcodePending_.store(false);
    }
    if (slicing_.load()) {
        ImGui::OpenPopup("Slicing Model");
        slicing_.store(false);
    }
    ImGui::SetNextWindowSize(ImVec2(400,350), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f,0.5f));
    if (ImGui::BeginPopupModal("Slicing Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* prompt = "Please wait";
        float ww = ImGui::GetWindowWidth();
        float tw = ImGui::CalcTextSize(prompt).x;
        ImGui::SetCursorPosX((ww - tw)*0.5f);
        ImGui::Text("%s", prompt);
        ImGui::Spacing();
        float fraction = slicingProgress_.load();
        const float radius = 120.f;
        const float thickness = 8.f;
        ImU32 fg = IM_COL32(75,175,255,255);
        ImU32 bg = IM_COL32(60,60,60,128);
        ImGui::SetCursorPosX((ww - radius*2.f)*0.5f);
        ImGui::ProgressBar("##slice_progress", fraction, radius, thickness, fg, bg);
        ImGui::Spacing();
        {
            std::lock_guard<std::mutex> lk(slicingMessageMutex_);
            std::string msg = slicingMessage_;
            float tw2 = ImGui::CalcTextSize(msg.c_str()).x;
            ImGui::SetCursorPosX((ww - tw2)*0.5f);
            ImGui::Text("%s", msg.c_str());
        }
        if (slicingDone_.load()) {
            ImGui::Spacing();
            float bw = 120.f;
            ImGui::SetCursorPosX((ww - bw)*0.5f);
            if (ImGui::Button("Close", ImVec2(bw,0))) {
                ImGui::CloseCurrentPopup();
                slicingDone_.store(false);
            }
        }
        ImGui::EndPopup();
    }
}


void UIManager::sliceActiveModel() {
    if (activeModel_ < 0 || activeModel_ >= static_cast<int>(modelManager_.Count()))
        return;

    slicingModelIndex_ = activeModel_;
    pendingStlPath_ = modelManager_.GetPath(activeModel_);
    std::filesystem::path base(pendingStlPath_);
    std::string name = base.stem().string();
    pendingResizedPath_ = (std::filesystem::path(GCODE_OUTPUT_DIR) / (name + "_resized.stl")).string();
    pendingGcodePath_ = (std::filesystem::path(GCODE_OUTPUT_DIR) / (name + ".gcode")).string();

    slicing_.store(true);
    slicingDone_.store(false);
    slicingProgress_.store(0.0f);

    std::thread([this]() {
        if (!std::filesystem::exists(MODEL_SETTINGS_FILE)) {
            std::lock_guard lk(slicingMessageMutex_);
            slicingMessage_ = "model_settings.json not found.";
        }
        modelManager_.ExportTransformedModel(slicingModelIndex_, pendingResizedPath_);
        float offX = renderer_ ? renderer_->GetBedHalfWidth()  : 0.f;
        float offY = renderer_ ? renderer_->GetBedHalfDepth() : 0.f;
        std::string cmd = std::string(CURA_ENGINE_EXE) +
                          " slice -j " + std::string(PRIMITIVE_PRINTER_SETTINGS_FILE) + " -j " +
                          std::string(BASE_PRINTER_SETTINGS_FILE) + " -j " + std::string(A1MINI_PRINTER_SETTINGS_FILE) +
                          " -j " + std::string(MODEL_SETTINGS_FILE) +
                          " -l \"" + pendingResizedPath_ + "\"" +
                          " -o \"" + pendingGcodePath_ + "\"";
        std::cout << cmd;
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe) {
            std::lock_guard lk(slicingMessageMutex_);
            slicingMessage_ = "Failed to start CuraEngine.";
            slicingDone_.store(true);
            slicingProgress_.store(1.0f);
            return;
        }
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') line.pop_back();
            {
                std::lock_guard lk(slicingMessageMutex_);
                slicingMessage_ = line;
            }
            std::smatch m;
            if (std::regex_search(line, m, std::regex("([0-9]+(?:\\.[0-9]+)?)%"))) {
                try {
                    float perc = std::stof(m[1].str());
                    slicingProgress_.store(perc / 100.f);
                } catch (...) {}
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
        if (ret == 0) {
            loadGcodePending_.store(true);
        }
    }).detach();
}

void UIManager::loadModel(std::string& modelPath) {
    try {
        activeModel_ = modelManager_.LoadModel(modelPath);
    } catch (const std::exception& e) {
        errorModalMessage_ = "Failed to load model: " + modelPath + "\n" + e.what();
        std::cout << errorModalMessage_ << std::endl;
        showErrorModal_ = true;
    }
}

void UIManager::UnloadModel(int idx) {
    modelManager_.UnloadModel(idx);
    if (activeModel_ == idx)
        activeModel_ = -1;
    else if (activeModel_ > idx)
        activeModel_--;
}


void UIManager::openRenderScene() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags f = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("DockspaceHost", nullptr, f);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(ImGui::GetID("MyMainDockspace"), ImVec2(0,0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void UIManager::openModelPropertiesDialog() {
    ImGui::Begin("Model Properties");
    if (activeModel_ >= 0 && activeModel_ < static_cast<int>(modelManager_.Count())) {
        ImGui::Text("Model Index: %d", activeModel_);
        glm::vec3 dims = modelManager_.GetDimensions(activeModel_);
        ImGui::Text("Real Dimensions (mm): %.2f x %.2f x %.2f", dims.x, dims.y, dims.z);
        ImGui::Separator();
        auto& tf = *modelManager_.GetTransform(activeModel_);
        bool changed = false;
        if (ImGui::BeginTable("TransformTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Translation");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##TransX", &tf.translation.x, 0.01f)) changed = true;
            ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##TransY", &tf.translation.y, 0.01f)) changed = true;
            ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##TransZ", &tf.translation.z, 0.01f)) changed = true;
            glm::vec3 euler = tf.getEulerAngles();
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Rotation");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##RotX", &euler.x, 0.5f)) { tf.setEulerAngles(euler); changed = true; }
            ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##RotY", &euler.y, 0.5f)) { tf.setEulerAngles(euler); changed = true; }
            ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##RotZ", &euler.z, 0.5f)) { tf.setEulerAngles(euler); changed = true; }
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Scale");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##ScaleX", &tf.scale.x, 0.01f, 0.001f, 100.f)) changed = true;
            ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##ScaleY", &tf.scale.y, 0.01f, 0.001f, 100.f)) changed = true;
            ImGui::TableSetColumnIndex(3); ImGui::SetNextItemWidth(-FLT_MIN); if (ImGui::DragFloat("##ScaleZ", &tf.scale.z, 0.01f, 0.001f, 100.f)) changed = true;
            ImGui::EndTable();
        }
        if (changed) {
            modelManager_.EnforceGridConstraint(activeModel_);
            modelManager_.UpdateDimensions(activeModel_);
        }
        ImGui::Separator();
        if (ImGui::Button("Reset Transform")) {
            glm::vec3 oC = modelManager_.GetModel(activeModel_)->center;
            glm::vec3 oMin = modelManager_.GetModel(activeModel_)->minBounds;
            glm::vec3 oMax = modelManager_.GetModel(activeModel_)->maxBounds;
            tf.rotationQuat = glm::quat(1.0f,0,0,0);
            glm::vec3 c[8];
            c[0] = tf.rotationQuat * glm::vec3(oMin.x,oMin.y,oMin.z);
            c[1] = tf.rotationQuat * glm::vec3(oMax.x,oMin.y,oMin.z);
            c[2] = tf.rotationQuat * glm::vec3(oMin.x,oMax.y,oMin.z);
            c[3] = tf.rotationQuat * glm::vec3(oMax.x,oMax.y,oMin.z);
            c[4] = tf.rotationQuat * glm::vec3(oMin.x,oMin.y,oMax.z);
            c[5] = tf.rotationQuat * glm::vec3(oMax.x,oMin.y,oMax.z);
            c[6] = tf.rotationQuat * glm::vec3(oMin.x,oMax.y,oMax.z);
            c[7] = tf.rotationQuat * glm::vec3(oMax.x,oMax.y,oMax.z);
            glm::vec3 newMin = c[0];
            for (int i=1;i<8;++i) newMin = glm::min(newMin,c[i]);
            glm::vec3 centerRot = tf.rotationQuat * oC;
            tf.translation = glm::vec3(-centerRot.x,-centerRot.y,-newMin.z);
            tf.scale = glm::vec3(1.0f);
            modelManager_.EnforceGridConstraint(activeModel_);
            modelManager_.UpdateDimensions(activeModel_);
        }
        ImGui::SameLine();
        if (ImGui::Button("Unload Model")) {
            UnloadModel(activeModel_);
        }
    } else {
        ImGui::Text("No model selected.");
    }
    if (gcodeModel_) {
        int layerCount = gcodeModel_->GetLayerCount();
        auto layerHeights = gcodeModel_->GetLayerHeights();
        if (currentGCodeLayer_ < -1) currentGCodeLayer_ = -1;
        if (currentGCodeLayer_ > layerCount-1) currentGCodeLayer_ = layerCount-1;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+10);
        ImGui::BeginGroup();
        ImGui::Text("G-code Layers:");
        if (layerCount > 0) {
            ImGui::Text("Z = %.2f mm (layer %d of %d)", layerHeights[(currentGCodeLayer_<0?0:currentGCodeLayer_)], (currentGCodeLayer_<0?0:currentGCodeLayer_), layerCount-1);
            ImGui::SliderInt("Layer", &currentGCodeLayer_, -1, layerCount-1, currentGCodeLayer_ < 0 ? "All" : "%d");
        } else {
            ImGui::Text("No layers found in G-code");
        }
        ImGui::EndGroup();
    }
    ImGui::End();
}

void UIManager::getActiveModel(glm::mat4& viewMatrix, const ImVec2& viewportScreenPos, const ImVec2& viewportSize) {
    float nearT = std::numeric_limits<float>::infinity();
    int pick = -1;
    ImVec2 mG = ImGui::GetMousePos();
    float rX = mG.x - viewportScreenPos.x;
    float rY = mG.y - viewportScreenPos.y;
    float nX = (2.f*rX)/viewportSize.x - 1.f;
    float nY = 1.f - (2.f*rY)/viewportSize.y;
    if (!renderer_ || viewportSize.x<=0 || viewportSize.y<=0) return;
    glm::mat4 iP = glm::inverse(renderer_->GetProjectionMatrix());
    glm::vec4 rE = iP * glm::vec4(nX,nY,-1,1);
    rE = glm::vec4(rE.x,rE.y,-1,0);
    glm::mat4 iV = glm::inverse(viewMatrix);
    glm::vec3 rO = glm::vec3(iV[3]);
    glm::vec3 rD = glm::normalize(glm::vec3(iV*rE));
    for (size_t i=0;i<modelManager_.Count();++i) {
        Model* mdl = modelManager_.GetModel(static_cast<int>(i));
        Transform* tr = modelManager_.GetTransform(static_cast<int>(i));
        if (!mdl || !tr) continue;
        float t;
        glm::mat4 mW = tr->getMatrix();
        glm::vec3 wC = glm::vec3(mW * glm::vec4(mdl->center,1));
        float mS = glm::max(tr->scale.x, glm::max(tr->scale.y, tr->scale.z));
        float wR = mdl->radius * mS;
        if (RayIntersectSphere(rO,rD,wC,wR,t) && t < nearT) {
            nearT = t; pick = static_cast<int>(i);
        }
    }
    activeModel_ = pick;
}

void UIManager::renderModels(glm::mat4&) {
    for (size_t i=0;i<modelManager_.Count();++i) {
        Model* m = modelManager_.GetModel(static_cast<int>(i));
        Shader* sh = modelManager_.GetShader(static_cast<int>(i));
        Transform* t = modelManager_.GetTransform(static_cast<int>(i));
        if (m && sh && t && renderer_ && sh->ID != 0) {
            renderer_->RenderModel(*m,*sh,*t);
        }
    }
}

void UIManager::finalizeSlicing() {
    try {
        auto gm = std::make_shared<GCodeModel>(pendingGcodePath_);
        if (renderer_) {
            glm::vec3 gcodeCenter = gm->GetCenter() -
                glm::vec3(renderer_->GetBedHalfWidth(), renderer_->GetBedHalfDepth(), 0.f);
            glm::vec3 modelCenter(0.f);
            if (slicingModelIndex_ >= 0) {
                Transform* tf = modelManager_.GetTransform(slicingModelIndex_);
                if (tf) modelCenter = tf->translation;
            }
            glm::vec3 offset = modelCenter - gcodeCenter;
            renderer_->SetGCodeOffset(offset);
            renderer_->SetGCodeModel(gm);
        }
        gcodeModel_ = gm;
        currentGCodeLayer_ = -1;
        UnloadModel(slicingModelIndex_);
        std::filesystem::remove(pendingResizedPath_);
        std::filesystem::remove(pendingStlPath_);
        std::filesystem::remove_all(std::string(OUTPUT_DIR));
    } catch (const std::exception &e) {
        std::lock_guard lk(slicingMessageMutex_);
        slicingMessage_ += std::string(" | Load failed: ") + e.what();
    }
}

