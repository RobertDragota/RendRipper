#include "FrameBuffer.h"
#include <iostream>

FrameBuffer::~FrameBuffer()
{
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    if (colorTex_) glDeleteTextures(1, &colorTex_);
    if (depthStencilRBO_) glDeleteRenderbuffers(1, &depthStencilRBO_);
}

void FrameBuffer::Init(int width, int height)
{
    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &colorTex_);
    glGenRenderbuffers(1, &depthStencilRBO_);
    Resize(width, height);
}

void FrameBuffer::Resize(int width, int height)
{
    if (!fbo_) Init(width, height); // ensure created
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glBindTexture(GL_TEXTURE_2D, colorTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex_, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRBO_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilRBO_);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer not complete!" << std::endl;
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void FrameBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
