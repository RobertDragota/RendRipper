#include "Shader.h"
#include "ShaderModule.h"
#include "IO_utility.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <iostream>

Shader::Shader(const char *vPath, const char *fPath, const char *gPath)
{
    ShaderModule v, f, g;
    if (vPath) v = ShaderModule::FromFile(GL_VERTEX_SHADER, vPath);
    if (fPath) f = ShaderModule::FromFile(GL_FRAGMENT_SHADER, fPath);
    if (gPath) g = ShaderModule::FromFile(GL_GEOMETRY_SHADER, gPath);
    *this = Shader(std::move(v), std::move(f), std::move(g));
}

Shader::Shader(ShaderModule &&vert, ShaderModule &&frag, ShaderModule &&geom)
{
    ID = glCreateProgram();
    if (vert.id())   glAttachShader(ID, vert.id());
    if (geom.id())   glAttachShader(ID, geom.id());
    if (frag.id())   glAttachShader(ID, frag.id());
    glLinkProgram(ID);
    int success = 0;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        glDeleteProgram(ID);
        ID = 0;
        throw std::runtime_error(std::string("Shader link error: ") + infoLog);
    }
}

Shader::~Shader()
{
    if (ID) {
        glDeleteProgram(ID);
        ID = 0;
    }
}

Shader::Shader(Shader &&o) noexcept : ID(o.ID), uniformLocationCache(std::move(o.uniformLocationCache))
{
    o.ID = 0;
}

Shader &Shader::operator=(Shader &&o) noexcept
{
    if (this != &o) {
        if (ID) glDeleteProgram(ID);
        ID = o.ID;
        uniformLocationCache = std::move(o.uniformLocationCache);
        o.ID = 0;
    }
    return *this;
}

void Shader::use() const noexcept
{
    if (ID) glUseProgram(ID);
}

int Shader::getUniformLocation(const std::string &name) const
{
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) return it->second;
    int loc = glGetUniformLocation(ID, name.c_str());
    uniformLocationCache[name] = loc;
    return loc;
}

bool Shader::hasUniform(const std::string &name) const
{
    if (!ID) return false;
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) return it->second != -1;
    return getUniformLocation(name) != -1;
}

void Shader::setBool(const std::string &n, bool v) const
{
    if (!ID) return;
    glUniform1i(getUniformLocation(n), static_cast<int>(v));
}

void Shader::setInt(const std::string &n, int v) const
{
    if (!ID) return;
    glUniform1i(getUniformLocation(n), v);
}

void Shader::setFloat(const std::string &n, float v) const
{
    if (!ID) return;
    glUniform1f(getUniformLocation(n), v);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    if (!ID) return;
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    if (!ID) return;
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string &n, const glm::vec4 &v) const
{
    if (!ID) return;
    glUniform4fv(getUniformLocation(n), 1, glm::value_ptr(v));
}

void Shader::setMat4(const std::string &n, const glm::mat4 &m) const
{
    if (!ID) return;
    glUniformMatrix4fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m));
}
