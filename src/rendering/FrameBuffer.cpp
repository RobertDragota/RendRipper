#include "FrameBuffer.h"
#include <iostream>

/**
 * @file FrameBuffer.cpp
 * @brief Implements a simple multisampled framebuffer wrapper.
 */

/** @brief Release all allocated GL objects. */
FrameBuffer::~FrameBuffer()
{
    if (msFbo_) glDeleteFramebuffers(1, &msFbo_);
    if (msColorTex_) glDeleteTextures(1, &msColorTex_);
    if (msDepthStencilRBO_) glDeleteRenderbuffers(1, &msDepthStencilRBO_);
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    if (colorTex_) glDeleteTextures(1, &colorTex_);
    if (depthStencilRBO_) glDeleteRenderbuffers(1, &depthStencilRBO_);
}

/** Initialize the framebuffer attachments. */
void FrameBuffer::Init(int width, int height, int samples)
{
    samples_ = samples;
    width_ = width;
    height_ = height;

    glGenFramebuffers(1, &msFbo_);
    glGenTextures(1, &msColorTex_);
    glGenRenderbuffers(1, &msDepthStencilRBO_);

    glGenFramebuffers(1, &fbo_);
    glGenTextures(1, &colorTex_);
    glGenRenderbuffers(1, &depthStencilRBO_);

    Resize(width, height);
}

/** Resize all framebuffer textures/buffers. */
void FrameBuffer::Resize(int width, int height)
{
    width_ = width;
    height_ = height;
    if (!fbo_ || !msFbo_) return;

    // multisample framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, msFbo_);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex_);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples_, GL_RGBA8, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex_, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, msDepthStencilRBO_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples_, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msDepthStencilRBO_);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "MS Framebuffer not complete!" << std::endl;

    // resolved framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glBindTexture(GL_TEXTURE_2D, colorTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
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

/** Bind the multisample buffer for rendering. */
void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, msFbo_ ? msFbo_ : fbo_);
}

/** Resolve multisample data and unbind. */
void FrameBuffer::Unbind()
{
    if (msFbo_) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msFbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
        glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
