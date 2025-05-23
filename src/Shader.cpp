#include "Shader.h"
#include "IO_utility.h" // Assuming this provides readFile

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <iostream> // For potential debug messages in hasUniform

Shader::Shader(const char *vPath, const char *fPath) {
    std::string vsCode = IO_utility::readFile(vPath);
    std::string fsCode = IO_utility::readFile(fPath);
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
        throw std::runtime_error(std::string("VERTEX SHADER ERROR (") + vPath + "):\n" + infoLog);
    }

    unsigned int fsID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsID, 1, &fSrc, nullptr);
    glCompileShader(fsID);
    glGetShaderiv(fsID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fsID, 512, nullptr, infoLog);
        glDeleteShader(vsID); // Clean up vertex shader if fragment fails
        throw std::runtime_error(std::string("FRAGMENT SHADER ERROR (") + fPath + "):\n" + infoLog);
    }

    ID = glCreateProgram();
    glAttachShader(ID, vsID);
    glAttachShader(ID, fsID);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        glDeleteShader(vsID); // Clean up shaders
        glDeleteShader(fsID);
        glDeleteProgram(ID); // Clean up program
        ID = 0; // Mark as invalid
        throw std::runtime_error(std::string("SHADER PROGRAM LINK ERROR (") + vPath + ", " + fPath + "):\n" + infoLog);
    }

    // Shaders are linked into the program; they are no longer needed individually
    glDetachShader(ID, vsID); // Detach before deleting is good practice, though not strictly required by all drivers
    glDetachShader(ID, fsID);
    glDeleteShader(vsID);
    glDeleteShader(fsID);
}

Shader::~Shader() {
    if (ID != 0) { // Check if ID is valid before deleting
        glDeleteProgram(ID);
        ID = 0;
    }
}

Shader::Shader(Shader &&o) noexcept
        : ID(o.ID), uniformLocationCache(std::move(o.uniformLocationCache)) {
    o.ID = 0; // Transfer ownership
}

Shader &Shader::operator=(Shader &&o) noexcept {
    if (this != &o) {
        if (ID != 0) {
            glDeleteProgram(ID);
        }
        ID = o.ID;
        uniformLocationCache = std::move(o.uniformLocationCache);
        o.ID = 0; // Transfer ownership
    }
    return *this;
}

void Shader::use() const noexcept {
    if (ID != 0) { // Only use if program ID is valid
        glUseProgram(ID);
    } else {
        // Optionally log an error or warning if trying to use an invalid shader
        // std::cerr << "Warning: Attempting to use an invalid shader program." << std::endl;
    }
}

int Shader::getUniformLocation(const std::string &name) const {
    if (ID == 0) return -1; // Cannot get location for an invalid program

    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) {
        return it->second;
    }

    // glGetUniformLocation should be called on the currently bound program,
    // but since 'use()' is separate, we assume the user calls use() before setting uniforms.
    // For robustness, one might call glUseProgram(ID) here if it's not too much overhead,
    // or ensure 'use()' is always called. The current design relies on the user calling 'use()'.
    int loc = glGetUniformLocation(ID, name.c_str());
    // Cache the result, even if it's -1 (uniform not found)
    uniformLocationCache[name] = loc;

    // Optional: Warn if uniform is not found the first time
    // if (loc == -1) {
    //     std::cerr << "Warning: Uniform '" << name << "' not found in shader program ID " << ID << std::endl;
    // }
    return loc;
}

bool Shader::hasUniform(const std::string& name) const {
    if (ID == 0) return false; // Invalid shader program cannot have uniforms

    // Check cache first
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) {
        return it->second != -1; // If cached, return true if location is not -1
    }

    // If not in cache, query OpenGL and then cache it
    // This will also populate the cache for subsequent getUniformLocation calls
    return getUniformLocation(name) != -1;
}


void Shader::setBool(const std::string &n, bool v) const {
    if (ID == 0) return;
    glUniform1i(getUniformLocation(n), static_cast<int>(v));
}

void Shader::setInt(const std::string &n, int v) const {
    if (ID == 0) return;
    glUniform1i(getUniformLocation(n), v);
}

void Shader::setFloat(const std::string &n, float v) const {
    if (ID == 0) return;
    glUniform1f(getUniformLocation(n), v);
}

void Shader::setMat4(const std::string &n, const glm::mat4 &m) const {
    if (ID == 0) return;
    glUniformMatrix4fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setVec4(const std::string &n, const glm::vec4 &v) const {
    if (ID == 0) return;
    glUniform4fv(getUniformLocation(n), 1, glm::value_ptr(v));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    if (ID == 0) return;
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}