#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"
#include "Transform.h"


class GizmoController {
public:
    GizmoController();
    void Manipulate(const glm::mat4& view,
                    const glm::mat4& proj,
                    Transform&       transform);

    ImGuizmo::OPERATION GetCurrentMode();

    void SetCurrentMode(ImGuizmo::OPERATION operation);

private:

    void ComputeRotationMatrix( glm::mat4 & transformMatrix);
    void ComputeTranslationMatrix( glm::mat4 & transformMatrix);
    void ComputeScaleMatrix( glm::mat4 & transformMatrix);

    ImGuizmo::OPERATION currentOp_;
    ImGuizmo::MODE      currentMode_;

    glm::mat3 rotation_{};
    glm::vec3 translation_{};
    glm::vec3 scale_{};
};
