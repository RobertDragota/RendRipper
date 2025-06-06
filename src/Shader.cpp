#include "Shader.h"
#include "IO_utility.h" // Provides FileIO helpers

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <iostream> // For potential debug messages in hasUniform

Shader::Shader(const char *vPath, const char *fPath, const char* gPath) {
    // 0) Parameter existence checks (no exceptions, just messages)
    bool hasVertex   = (vPath != nullptr);
    bool hasFragment = (fPath != nullptr);
    bool hasGeometry = (gPath != nullptr);

    if (!hasVertex) {
        std::cerr << "[Vertex shader] is not present\n";
    }
    if (!hasFragment) {
        std::cerr << "[Fragment shader] is not present\n";
    }
    if (!hasGeometry) {
        std::cerr << "[Geometry shader] is not present\n";
    }

    int success;
    char infoLog[512];

    unsigned int vsID = 0, fsID = 0, gsID = 0;

    // 1. Compile vertex shader (if provided)
    if (hasVertex) {
        std::string vsCode = IO_utility::FileIO::ReadTextFile(vPath);
        const char *vSrc = vsCode.c_str();
        vsID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vsID, 1, &vSrc, nullptr);
        glCompileShader(vsID);
        glGetShaderiv(vsID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vsID, 512, nullptr, infoLog);
            glDeleteShader(vsID);
            throw std::runtime_error(std::string("VERTEX SHADER ERROR (") + vPath + "):\n" + infoLog);
        }
    }

    // 2. Compile fragment shader (if provided)
    if (hasFragment) {
        std::string fsCode = IO_utility::FileIO::ReadTextFile(fPath);
        const char *fSrc = fsCode.c_str();
        fsID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fsID, 1, &fSrc, nullptr);
        glCompileShader(fsID);
        glGetShaderiv(fsID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fsID, 512, nullptr, infoLog);
            if (hasVertex) glDeleteShader(vsID);
            glDeleteShader(fsID);
            throw std::runtime_error(std::string("FRAGMENT SHADER ERROR (") + fPath + "):\n" + infoLog);
        }
    }

    // 3. Compile geometry shader (if provided)
    if (hasGeometry) {
        std::string gsCode = IO_utility::FileIO::ReadTextFile(gPath);
        const char *gSrc = gsCode.c_str();
        gsID = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gsID, 1, &gSrc, nullptr);
        glCompileShader(gsID);
        glGetShaderiv(gsID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(gsID, 512, nullptr, infoLog);
            if (hasVertex)   glDeleteShader(vsID);
            if (hasFragment) glDeleteShader(fsID);
            glDeleteShader(gsID);
            throw std::runtime_error(std::string("GEOMETRY SHADER ERROR (") + gPath + "):\n" + infoLog);
        }
    }

    // 4. Link shaders into program (attach only those that were compiled)
    ID = glCreateProgram();
    if (hasVertex)   glAttachShader(ID, vsID);
    if (hasGeometry) glAttachShader(ID, gsID);
    if (hasFragment) glAttachShader(ID, fsID);

    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        if (hasVertex)   glDeleteShader(vsID);
        if (hasGeometry) glDeleteShader(gsID);
        if (hasFragment) glDeleteShader(fsID);
        glDeleteProgram(ID);
        ID = 0;
        throw std::runtime_error(
            std::string("SHADER PROGRAM LINK ERROR (") +
            (hasVertex ? std::string(vPath) : std::string("no-vertex")) + ", " +
            (hasGeometry ? std::string(gPath) : std::string("no-geometry")) + ", " +
            (hasFragment ? std::string(fPath) : std::string("no-fragment")) + "):\n" + infoLog
        );
    }

    // 5. Detach and delete individual shaders
    if (hasVertex) {
        glDetachShader(ID, vsID);
        glDeleteShader(vsID);
    }
    if (hasGeometry) {
        glDetachShader(ID, gsID);
        glDeleteShader(gsID);
    }
    if (hasFragment) {
        glDetachShader(ID, fsID);
        glDeleteShader(fsID);
    }
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

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    if (ID == 0) return;
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
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
