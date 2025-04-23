#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"

struct Transform {
    glm::vec3 translation{0.0f};
    glm::quat rotationQuat{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    // for UI: get/set Eulers in degrees
    glm::vec3 getEulerAngles() const {
        return glm::degrees(glm::eulerAngles(rotationQuat));
    }
    void setEulerAngles(const glm::vec3& eulerDeg) {
        rotationQuat = glm::quat(glm::radians(eulerDeg));
    }
};

class GizmoController {
public:
    GizmoController();
    void Manipulate(const glm::mat4& view,
                    const glm::mat4& proj,
                    Transform&       transform);

private:

    void ComputeRotationMatrix( glm::mat4 & transformMatrix);
    void ComputeTranslationMatrix( glm::mat4 & transformMatrix);
    void ComputeScaleMatrix( glm::mat4 & transformMatrix);

    ImGuizmo::OPERATION currentOp_;
    ImGuizmo::MODE      currentMode_;

    glm::mat3 rotation_;
    glm::vec3 translation_;
    glm::vec3 scale_;
};
