#include "ShaderModule.h"
#include "IO_utility.h"
#include <stdexcept>

/**
 * @file ShaderModule.cpp
 * @brief Compiles individual shader stages.
 */

/** Compile a shader module from source code. */
ShaderModule::ShaderModule(GLenum type, const std::string &source)
{
    id_ = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id_, 1, &src, nullptr);
    glCompileShader(id_);
    int success = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id_, 512, nullptr, infoLog);
        glDeleteShader(id_);
        id_ = 0;
        throw std::runtime_error(std::string("Shader compile error: ") + infoLog);
    }
}

/** Load shader source from a file and compile it. */
ShaderModule ShaderModule::FromFile(GLenum type, const std::string &path)
{
    std::string code = IO_utility::FileIO::ReadTextFile(path);
    return {type, code};
}

/** Destructor deletes the GL shader object. */
ShaderModule::~ShaderModule()
{
    if (id_)
        glDeleteShader(id_);
}

/** Move constructor. */
ShaderModule::ShaderModule(ShaderModule &&other) noexcept : id_(other.id_)
{
    other.id_ = 0;
}

/** Move assignment. */
ShaderModule &ShaderModule::operator=(ShaderModule &&other) noexcept
{
    if (this != &other) {
        if (id_)
            glDeleteShader(id_);
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

