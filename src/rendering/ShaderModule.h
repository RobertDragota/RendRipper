#pragma once

#include <string>
#include <glad/glad.h>

/**
 * @brief Represents a compiled OpenGL shader stage.
 */
class ShaderModule {
public:
    ShaderModule() = default;
    ShaderModule(GLenum type, const std::string &source);
    static ShaderModule FromFile(GLenum type, const std::string &path);
    ~ShaderModule();

    ShaderModule(ShaderModule &&other) noexcept;
    ShaderModule &operator=(ShaderModule &&other) noexcept;

    ShaderModule(const ShaderModule &) = delete;
    ShaderModule &operator=(const ShaderModule &) = delete;

    /// Access the underlying shader object id.
    GLuint id() const { return id_; }

private:
    GLuint id_ = 0;
};

