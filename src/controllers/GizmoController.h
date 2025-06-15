#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"
#include <memory>
#include "Transform.h"

/**
 * @brief Interface for gizmo operations applied to transformations.
 */
class IGizmoOperation
{
public:
    virtual ~IGizmoOperation() = default;

    /**
     * @brief Apply the resulting matrix to the provided transform.
     * @param matrix Manipulated matrix from ImGuizmo.
     * @param transform Transform object to modify.
     */
    virtual void Apply(const glm::mat4 &matrix, ITransform &transform) = 0;
};

/**
 * @brief Controller responsible for manipulating transforms via ImGuizmo.
 */
class GizmoController
{
public:
    /**
     * @brief Construct a new GizmoController instance.
     */
    GizmoController();

    /**
     * @brief Display and manage the gizmo widget for the given transform.
     * @param view View matrix used for ImGuizmo.
     * @param proj Projection matrix used for ImGuizmo.
     * @param transform Transform to modify by the gizmo.
     */
    void Manipulate
    (
        const glm::mat4 &view,
        const glm::mat4 &proj,
        ITransform &transform
    );

    /**
     * @brief Retrieve the current gizmo operation mode.
     */
    ImGuizmo::OPERATION GetCurrentMode() const;

    /**
     * @brief Set the current gizmo operation mode.
     */
    void SetCurrentMode(ImGuizmo::OPERATION operation);

private:
    /**
     * @brief Update internal operation implementation according to mode.
     */
    void updateOperation();

    ImGuizmo::OPERATION currentOp_;
    ImGuizmo::MODE currentMode_;

    std::unique_ptr<IGizmoOperation> operation_;
};
