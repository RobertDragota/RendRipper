#include "GizmoController.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace
{

    /**
     * @brief Break down a transformation matrix into translation, scale and rotation.
     * @param matrix Matrix to decompose.
     * @param translation Output translation vector.
     * @param scale Output scale vector.
     * @param rotation Output rotation quaternion.
     */
    void DecomposeMatrix
    (
        const glm::mat4 &matrix,
        glm::vec3 &translation,
        glm::vec3 &scale,
        glm::quat &rotation
    )
    {
        translation = glm::vec3(matrix[3]);
        scale.x = glm::length(glm::vec3(matrix[0]));
        scale.y = glm::length(glm::vec3(matrix[1]));
        scale.z = glm::length(glm::vec3(matrix[2]));
        glm::mat3 rot;
        rot[0] = glm::vec3(matrix[0]) / scale.x;
        rot[1] = glm::vec3(matrix[1]) / scale.y;
        rot[2] = glm::vec3(matrix[2]) / scale.z;
        rotation = glm::quat_cast(rot);
    }

    /**
     * @brief Default gizmo operation that writes decomposed matrix to a transform.
     */
    class BasicOperation : public IGizmoOperation
    {
    public:
        /**
         * @brief Apply the decomposed matrix values to the given transform.
         */
        void Apply(const glm::mat4 &m, ITransform &t) override
        {
            glm::vec3 tr, sc;
            glm::quat rot;
            DecomposeMatrix(m, tr, sc, rot);
            t.setTranslation(tr);
            t.setScale(sc);
            t.setRotationQuat(rot);
        }
    };

} // namespace

/**
 * @brief Construct a new GizmoController with default mode and operation.
 */
GizmoController::GizmoController()
    : currentOp_(ImGuizmo::TRANSLATE),
      currentMode_(ImGuizmo::LOCAL),
      operation_(std::make_unique<BasicOperation>())
{
}

/**
 * @brief Manipulate the provided transform using ImGuizmo.
 */
void GizmoController::Manipulate
(
    const glm::mat4 &view,
    const glm::mat4 &proj,
    ITransform &transform
)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImVec2 windowPos = ImGui::GetWindowPos();
    float windowWidth = ImGui::GetWindowWidth();
    float windowHeight = ImGui::GetWindowHeight();
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowWidth, windowHeight);

    // current model matrix
    glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.getTranslation())
                      * glm::toMat4(transform.getRotationQuat())
                      * glm::scale(glm::mat4(1.0f), transform.getScale());

    float v[16], p[16], m[16];
    memcpy(v, glm::value_ptr(view), sizeof(v));
    memcpy(p, glm::value_ptr(proj), sizeof(p));
    memcpy(m, glm::value_ptr(model), sizeof(m));

    // optional hotkeys
    if (ImGui::IsKeyPressed(ImGuiKey_T))
        currentOp_ = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
        currentOp_ = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_S))
        currentOp_ = ImGuizmo::SCALE;
    updateOperation();

    if (ImGuizmo::Manipulate(v, p, currentOp_, currentMode_, m, nullptr, nullptr))
        {
        glm::mat4 transformMatrix = glm::make_mat4(m);
        if (operation_)
            operation_->Apply(transformMatrix, transform);
        }
}

/**
 * @brief Get the current ImGuizmo operation type.
 */
ImGuizmo::OPERATION GizmoController::GetCurrentMode() const
{
    return currentOp_;
}

/**
 * @brief Set a new ImGuizmo operation type.
 */
void GizmoController::SetCurrentMode(ImGuizmo::OPERATION operation)
{
    currentOp_ = operation;
    updateOperation();
}

/**
 * @brief Create a new operation implementation matching the current mode.
 */
void GizmoController::updateOperation()
{
    operation_ = std::make_unique<BasicOperation>();
}
