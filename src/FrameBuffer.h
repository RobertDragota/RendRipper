#pragma once
#include <glad/glad.h>

class FrameBuffer {
public:
    FrameBuffer() = default;
    ~FrameBuffer();

    void Init(int width, int height);
    void Resize(int width, int height);
    void Bind();
    void Unbind();

    GLuint GetColorTexture() const { return colorTex_; }

private:
    GLuint fbo_ = 0;
    GLuint colorTex_ = 0;
    GLuint depthStencilRBO_ = 0;
};
