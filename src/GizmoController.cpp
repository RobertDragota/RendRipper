#include "GizmoController.h"
#include "glm/gtc/type_ptr.hpp"
#include <imgui.h>
#include <ImGuizmo.h>

GizmoController::GizmoController()
        : currentOp_(ImGuizmo::TRANSLATE)
        , currentMode_(ImGuizmo::WORLD)
{}

void GizmoController::Manipulate(const glm::mat4& view,
                                 const glm::mat4& proj,
                                 Transform&       t)
{
    // draw into current window drawlist (over the image)
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 sz = ImGui::GetWindowSize();
    ImGuizmo::SetRect(wp.x, wp.y, sz.x, sz.y);

    // build T * R * S matrix
    glm::mat4 M = glm::translate(glm::mat4(1.0f), t.translation);
    M = glm::rotate(M, glm::radians(t.rotation.x), glm::vec3(1,0,0));
    M = glm::rotate(M, glm::radians(t.rotation.y), glm::vec3(0,1,0));
    M = glm::rotate(M, glm::radians(t.rotation.z), glm::vec3(0,0,1));
    M = glm::scale   (M, glm::vec3(t.scale));

    float v[16], p[16], m[16];
    memcpy(v, glm::value_ptr(view), sizeof(v));
    memcpy(p, glm::value_ptr(proj), sizeof(p));
    memcpy(m, glm::value_ptr(M),    sizeof(m));

    // shortcuts
    if (ImGui::IsKeyPressed(ImGuiKey_T)) currentOp_ = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R)) currentOp_ = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_S)) currentOp_ = ImGuizmo::SCALE;

    if (ImGuizmo::Manipulate(v, p,
                             currentOp_, currentMode_,
                             m, nullptr, nullptr))
    {
        float trans[3], rot[3], sc[3];
        ImGuizmo::DecomposeMatrixToComponents(m,
                                              trans, rot, sc);
        t.translation = { trans[0], trans[1], trans[2] };
        t.rotation    = { rot[0],   rot[1],   rot[2]   };
        t.scale       = sc[0];
    }
}
