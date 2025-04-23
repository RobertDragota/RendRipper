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
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

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
        glm::mat4 newM = glm::make_mat4(m);
        // translation
        auto newTrans = glm::vec3(newM[3]);
        // basis columns
        auto c0 = glm::vec3(newM[0]);
        auto c1 = glm::vec3(newM[1]);
        auto c2 = glm::vec3(newM[2]);
        float sx = glm::length(c0);
        float sy = glm::length(c1);
        float sz = glm::length(c2);
        // normalized rotation matrix
        glm::mat3 rotM;
        rotM[0] = c0 / sx;
        rotM[1] = c1 / sy;
        rotM[2] = c2 / sz;
        glm::quat newQuat = glm::quat_cast(rotM);

        transform.translation  = newTrans;
        transform.scale        = glm::vec3(sx, sy, sz);
        transform.rotationQuat = newQuat;
    }
}