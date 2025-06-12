#pragma once
#include <glad/glad.h>

class FrameBuffer {
public:
    FrameBuffer() = default;
    ~FrameBuffer();

    void Init(int width, int height, int samples = 4);
    void Resize(int width, int height);
    void Bind();
    void Unbind();

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
