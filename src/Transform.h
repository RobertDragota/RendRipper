#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp> // For glm::toMat4, glm::eulerAngles

struct Transform {
    glm::vec3 translation{0.0f, 0.0f, 0.0f};
    glm::quat rotationQuat{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};


    [[nodiscard]] glm::mat4 getMatrix() const {
        glm::mat4 transMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotMatrix = glm::toMat4(rotationQuat);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        return transMatrix * rotMatrix * scaleMatrix;
    }


    [[nodiscard]] glm::vec3 getEulerAngles() const {
        return glm::degrees(glm::eulerAngles(rotationQuat));
    }

    void setEulerAngles(const glm::vec3& eulerAnglesDegrees) {
        rotationQuat = glm::quat(glm::radians(eulerAnglesDegrees));
    }
};