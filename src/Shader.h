// Shader.h
#pragma once

#include <string>
#include <glm/glm.hpp>
#include <unordered_map> // For uniformLocationCache

class Shader {
public:
    unsigned int ID = 0;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Move constructor and assignment
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // Disable copy constructor and assignment
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const noexcept;

    // Utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    // New function to check if a uniform exists
    bool hasUniform(const std::string& name) const;

private:
    // Uniform location caching
    mutable std::unordered_map<std::string, int> uniformLocationCache;
    int getUniformLocation(const std::string& name) const;
};