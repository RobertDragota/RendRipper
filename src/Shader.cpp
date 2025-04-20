#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

static std::string readFile(const char *path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error(std::string("Cannot open file: ") + path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

Shader::Shader(const char *vPath, const char *fPath) {
    // 1) Citește și compilează shaderele
    std::string vsCode = readFile(vPath);
    std::string fsCode = readFile(fPath);
    const char *vSrc = vsCode.c_str();
    const char *fSrc = fsCode.c_str();

    unsigned int vsID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsID, 1, &vSrc, nullptr);
    glCompileShader(vsID);
    int success;
    char infoLog[512];
    glGetShaderiv(vsID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vsID, 512, nullptr, infoLog);
        throw std::runtime_error(std::string("VERTEX SHADER ERROR:\n") + infoLog);
    }

    unsigned int fsID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsID, 1, &fSrc, nullptr);
    glCompileShader(fsID);
    glGetShaderiv(fsID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fsID, 512, nullptr, infoLog);
        glDeleteShader(vsID);
        throw std::runtime_error(std::string("FRAGMENT SHADER ERROR:\n") + infoLog);
    }

    // 2) Link program
    ID = glCreateProgram();
    glAttachShader(ID, vsID);
    glAttachShader(ID, fsID);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        glDeleteShader(vsID);
        glDeleteShader(fsID);
        throw std::runtime_error(std::string("SHADER PROGRAM LINK ERROR:\n") + infoLog);
    }

    // 3) Curățenie
    glDeleteShader(vsID);
    glDeleteShader(fsID);
}

Shader::~Shader() {
    if (ID) glDeleteProgram(ID);
}

Shader::Shader(Shader &&o) noexcept
        : ID(o.ID), uniformLocationCache(std::move(o.uniformLocationCache)) {
    o.ID = 0;
}

Shader &Shader::operator=(Shader &&o) noexcept {
    if (this != &o) {
        if (ID) glDeleteProgram(ID);
        ID = o.ID;
        uniformLocationCache = std::move(o.uniformLocationCache);
        o.ID = 0;
    }
    return *this;
}

void Shader::use() const noexcept {
    glUseProgram(ID);
}

int Shader::getUniformLocation(const std::string &name) const {
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end())
        return it->second;
    int loc = glGetUniformLocation(ID, name.c_str());
    uniformLocationCache[name] = loc;
    return loc;
}

void Shader::setBool(const std::string &n, bool v) const { glUniform1i(getUniformLocation(n), (int) v); }

void Shader::setInt(const std::string &n, int v) const { glUniform1i(getUniformLocation(n), v); }

void Shader::setFloat(const std::string &n, float v) const { glUniform1f(getUniformLocation(n), v); }

void Shader::setMat4(const std::string &n, const glm::mat4 &m) const {
    glUniformMatrix4fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setVec4(const std::string &n, const glm::vec4 &v) const {
    glUniform4fv(getUniformLocation(n), 1, glm::value_ptr(v));
}
