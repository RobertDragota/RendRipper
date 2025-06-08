#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp> // For glm::toMat4, glm::eulerAngles

class ITransform {
public:
    virtual ~ITransform() = default;
    virtual glm::vec3 getTranslation() const = 0;
    virtual void setTranslation(const glm::vec3 &t) = 0;
    virtual glm::quat getRotationQuat() const = 0;
    virtual void setRotationQuat(const glm::quat &q) = 0;
    virtual glm::vec3 getScale() const = 0;
    virtual void setScale(const glm::vec3 &s) = 0;
    [[nodiscard]] virtual glm::mat4 getMatrix() const = 0;
};

struct Transform : public ITransform {
    glm::vec3 translation{0.0f, 0.0f, 0.0f};
    glm::quat rotationQuat{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};


    [[nodiscard]] glm::mat4 getMatrix() const override {
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

    glm::vec3 getTranslation() const override { return translation; }
    void setTranslation(const glm::vec3 &t) override { translation = t; }
    glm::quat getRotationQuat() const override { return rotationQuat; }
    void setRotationQuat(const glm::quat &q) override { rotationQuat = q; }
    glm::vec3 getScale() const override { return scale; }
    void setScale(const glm::vec3 &s) override { scale = s; }
};
