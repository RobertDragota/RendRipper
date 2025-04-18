#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const;
    void setBool(const std::string& name, bool v) const;
    void setInt(const std::string& name, int v) const;
    void setFloat(const std::string& name, float v) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec4(const std::string& name, const glm::vec4& vec) const;
};