#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class CameraController {
public:
    float yaw{0.f};
    float pitch{30.f};
    float distance{500.f};

    glm::mat4 GetViewMatrix(glm::vec3 &cameraWorldPos) const {
        float rP = glm::radians(pitch);
        float rY = glm::radians(yaw);
        cameraWorldPos.x = distance * cos(rP) * cos(rY);
        cameraWorldPos.y = distance * cos(rP) * sin(rY);
        cameraWorldPos.z = distance * sin(rP);
        glm::vec3 piv(0.f);
        glm::vec3 up(0.f, 0.f, 1.f);
        return glm::lookAt(cameraWorldPos + piv, piv, up);
    }
};
