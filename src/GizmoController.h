#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

struct Transform {
    glm::vec3 translation{0.0f};
    glm::vec3 rotation{0.0f};
    float      scale{1.0f};
};

class GizmoController {
public:
    GizmoController();
    void Manipulate(const glm::mat4& view,
                    const glm::mat4& proj,
                    Transform&       t);

private:
    ImGuizmo::OPERATION currentOp_;
    ImGuizmo::MODE      currentMode_;
};
