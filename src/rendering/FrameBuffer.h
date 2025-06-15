#pragma once
#include <glad/glad.h>

/**
 * @brief Wrapper for a multisampled framebuffer with resolve step.
 */
class FrameBuffer {
public:
    FrameBuffer() = default;
    ~FrameBuffer();

    /** @brief Allocate framebuffer textures and renderbuffers. */
    void Init(int width, int height, int samples = 4);
    /** @brief Resize internal attachments. */
    void Resize(int width, int height);
    /** @brief Bind for rendering. */
    void Bind();
    /** @brief Resolve multisample and unbind. */
    void Unbind();

    /** @brief Access resolved color texture. */
    GLuint GetColorTexture() const { return colorTex_; }

private:
    GLuint fbo_ = 0; // resolved framebuffer
    GLuint colorTex_ = 0;
    GLuint depthStencilRBO_ = 0;

    GLuint msFbo_ = 0; // multisample framebuffer
    GLuint msColorTex_ = 0;
    GLuint msDepthStencilRBO_ = 0;

    int width_ = 0;
    int height_ = 0;
    int samples_ = 4;
};
