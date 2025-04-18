#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char *vPath, const char *fPath) {
    std::ifstream vFile(vPath), fFile(fPath);
    if (!vFile || !fFile) throw std::runtime_error("Shader file not found");
    std::stringstream vs, fs;
    vs << vFile.rdbuf();
    fs << fFile.rdbuf();
    std::string vCode = vs.str(), fCode = fs.str();
    const char *vSrc = vCode.c_str();
    const char *fSrc = fCode.c_str();
    unsigned int vsID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsID, 1, &vSrc, nullptr);
    glCompileShader(vsID);
    int ok;
    char log[512];
    glGetShaderiv(vsID, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(vsID, 512, nullptr, log);
        std::cerr << "VERTEX ERROR:\n" << log;
    }
    unsigned int fsID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsID, 1, &fSrc, nullptr);
    glCompileShader(fsID);
    glGetShaderiv(fsID, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(fsID, 512, nullptr, log);
        std::cerr << "FRAG ERROR:\n" << log;
    }
    ID = glCreateProgram();
    glAttachShader(ID, vsID);
    glAttachShader(ID, fsID);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(ID, 512, nullptr, log);
        std::cerr << "LINK ERROR:\n" << log;
    }
    glDeleteShader(vsID);
    glDeleteShader(fsID);
}

void Shader::use() const { glUseProgram(ID); }

void Shader::setBool(const std::string &n, bool v) const { glUniform1i(glGetUniformLocation(ID, n.c_str()), (int) v); }

void Shader::setInt(const std::string &n, int v) const { glUniform1i(glGetUniformLocation(ID, n.c_str()), v); }

void Shader::setFloat(const std::string &n, float v) const { glUniform1f(glGetUniformLocation(ID, n.c_str()), v); }

void Shader::setMat4(const std::string &n, const glm::mat4 &m) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setVec4(const std::string &n, const glm::vec4 &v) const {
    glUniform4fv(glGetUniformLocation(ID, n.c_str()), 1, glm::value_ptr(v));
}
