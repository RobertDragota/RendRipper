#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"
#include <memory>
#include "Transform.h"

class IGizmoOperation
{
public:
    virtual ~IGizmoOperation() = default;

    virtual void Apply(const glm::mat4 &matrix, ITransform &transform) = 0;
};

class GizmoController
{
public:
    GizmoController();

    void Manipulate
    (
        const glm::mat4 &view,
        const glm::mat4 &proj,
        ITransform &transform
    );

    ImGuizmo::OPERATION GetCurrentMode() const;

    void SetCurrentMode(ImGuizmo::OPERATION operation);

    void SetOffset(const glm::vec3 &off) { offset_ = off; }

private:
    void updateOperation();

    ImGuizmo::OPERATION currentOp_;
    ImGuizmo::MODE currentMode_;

    std::unique_ptr<IGizmoOperation> operation_;
    glm::vec3 offset_{0.0f};
};
