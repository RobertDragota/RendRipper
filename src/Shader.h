#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

/**
 * Wrapper RAII pentru un program OpenGL:
 * - compilează/linkează shadere
 * - cache‑uiesc locațiile de uniforme
 * - șterg programul la destructor
 */
class Shader {
public:
    Shader(const char *vertexPath, const char *fragmentPath);

    ~Shader();

    // non‑copyable
    Shader(const Shader &) = delete;

    Shader &operator=(const Shader &) = delete;

    // movable
    Shader(Shader &&other) noexcept;

    Shader &operator=(Shader &&other) noexcept;

    void use() const noexcept;

    // setters eficiente (cache uniform locations)
    void setBool(const std::string &name, bool value) const;

    void setInt(const std::string &name, int value) const;

    void setFloat(const std::string &name, float value) const;

    void setMat4(const std::string &name, const glm::mat4 &mat) const;

    void setVec4(const std::string &name, const glm::vec4 &vec) const;

    void setVec3(const std::string &name, const glm::vec3 &value) const;

private:
    unsigned int ID;
    mutable std::unordered_map<std::string, int> uniformLocationCache;

    int getUniformLocation(const std::string &name) const;

};
