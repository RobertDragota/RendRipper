#include "GizmoController.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

GizmoController::GizmoController()
        : currentOp_(ImGuizmo::TRANSLATE),
          currentMode_(ImGuizmo::LOCAL) {}

void GizmoController::Manipulate(const glm::mat4 &view,
                                 const glm::mat4 &proj,
                                 Transform &transform) {
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImVec2 windowPos = ImGui::GetWindowPos();
    float windowWidth = ImGui::GetWindowWidth();
    float windowHeight = ImGui::GetWindowHeight();
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowWidth, windowHeight);

    // current model matrix
    glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.translation)
                      * glm::toMat4(transform.rotationQuat)
                      * glm::scale(glm::mat4(1.0f), transform.scale);

    float v[16], p[16], m[16];
    memcpy(v, glm::value_ptr(view), sizeof(v));
    memcpy(p, glm::value_ptr(proj), sizeof(p));
    memcpy(m, glm::value_ptr(model), sizeof(m));

    // optional hotkeys
    if (ImGui::IsKeyPressed(ImGuiKey_T)) currentOp_ = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R)) currentOp_ = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_S)) currentOp_ = ImGuizmo::SCALE;

    if (ImGuizmo::Manipulate(v, p, currentOp_, currentMode_, m, nullptr, nullptr)) {
        // read back
        glm::mat4 transformMatrix = glm::make_mat4(m);

        ComputeScaleMatrix(transformMatrix);
        ComputeRotationMatrix(transformMatrix);
        ComputeTranslationMatrix(transformMatrix);

        transform.translation = translation_;
        transform.scale = scale_;
        transform.rotationQuat = glm::quat_cast(rotation_);
    }
}

void GizmoController::ComputeRotationMatrix(glm::mat4 &transformMatrix) {

    auto pitch = glm::vec3(transformMatrix[0]) / scale_[0];
    auto yaw = glm::vec3(transformMatrix[1]) / scale_[1];
    auto roll = glm::vec3(transformMatrix[2]) / scale_[2];

    rotation_[0] = pitch;
    rotation_[1] = yaw;
    rotation_[2] = roll;

}

void GizmoController::ComputeTranslationMatrix(glm::mat4 &transformMatrix) {
    translation_ = glm::vec3(transformMatrix[3]);
}

void GizmoController::ComputeScaleMatrix(glm::mat4 &transformMatrix) {
    scale_[0] = glm::length(glm::vec3(transformMatrix[0]));
    scale_[1] = glm::length(glm::vec3(transformMatrix[1]));
    scale_[2] = glm::length(glm::vec3(transformMatrix[2]));
}

ImGuizmo::OPERATION GizmoController::GetCurrentMode() {
    return currentOp_;
}

void GizmoController::SetCurrentMode(ImGuizmo::OPERATION operation) {
    currentOp_ = operation;
}
