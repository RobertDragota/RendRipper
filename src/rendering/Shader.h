// Shader.h
#pragma once

#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>

class ShaderModule;

class Shader {
public:
    unsigned int ID = 0;

    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    Shader(ShaderModule&& vertex, ShaderModule&& fragment, ShaderModule&& geometry);
    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const noexcept;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    bool hasUniform(const std::string& name) const;

private:
    mutable std::unordered_map<std::string, int> uniformLocationCache;
    int getUniformLocation(const std::string& name) const;
};
