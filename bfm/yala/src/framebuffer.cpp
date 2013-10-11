#include <framebuffer.h>
#include <device.h>

//--------------------------------------------------------
//--------------------------------------------------------

#pragma region RenderTarget

RenderTarget::RenderTarget(size_t width, size_t height)
{
  texture.width = width;
  texture.height = height;
}

RenderTarget::~RenderTarget()
{
}

void RenderTarget::SetAsSource()
{
}

void RenderTarget::SetAsDestination()
{
}

#pragma endregion RenderTarget

//--------------------------------------------------------
//--------------------------------------------------------

#pragma region ColourRenderTarget

ColourRenderTarget::ColourRenderTarget(size_t width, size_t height)
  : RenderTarget(width, height)
{
  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
  glBindTexture(GL_TEXTURE_2D, texture.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA8, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glSamplerParameteri(texture.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(texture.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(texture.sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(texture.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

ColourRenderTarget::~ColourRenderTarget()
{
}

#pragma endregion ColourRenderTarget

//--------------------------------------------------------
//--------------------------------------------------------

#pragma region DepthRenderTarget
DepthRenderTarget::DepthRenderTarget(size_t width, size_t height)
  : RenderTarget(width, height)
{
  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
  glBindTexture(GL_TEXTURE_2D, texture.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT32F, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glSamplerParameteri(texture.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(texture.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(texture.sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(texture.sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

DepthRenderTarget::~DepthRenderTarget()
{
}

#pragma endregion DepthRenderTarget

//--------------------------------------------------------
//--------------------------------------------------------

#pragma region FrameBuffer

FrameBuffer::FrameBuffer()
{
  glGenFramebuffers(1, &handle);
}

//--------------------------------------------------------
FrameBuffer::~FrameBuffer()
{
  glDeleteFramebuffers(1, &handle);
}

//--------------------------------------------------------
void FrameBuffer::Enable()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
}

//--------------------------------------------------------
void FrameBuffer::Disable()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//--------------------------------------------------------
void FrameBuffer::AttachRenderTarget(boost::shared_ptr<DepthRenderTarget> depthRenderTarget)
{
  this->depthRenderTarget = depthRenderTarget;
  Enable();
  if (this->depthRenderTarget)
  {
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthRenderTarget->texture.texture, 0);
  }
  else
  {
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
  }
  Disable();
}

//--------------------------------------------------------
void FrameBuffer::AttachRenderTarget(size_t count, boost::shared_ptr<ColourRenderTarget> colourRenderTargets[])
{
  Enable();
  for (size_t i = 0; i < glm::min(count, MaxColourRenderTargets); ++i)
  {
    this->colourRenderTargets[i] = colourRenderTargets[i];
    if (this->colourRenderTargets[i])
    {
      glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colourRenderTargets[i]->texture.texture, 0);
    }
    else
    {
      glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 0, 0);
    }
  }
  Disable();
}

//--------------------------------------------------------
void FrameBuffer::AttachRenderTarget(size_t slot, boost::shared_ptr<ColourRenderTarget> colourRenderTarget)
{
  if (slot < MaxColourRenderTargets)
  {
    this->colourRenderTargets[slot] = colourRenderTarget;
    Enable();
    if (this->colourRenderTargets[slot])
    {
      glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, colourRenderTarget->texture.texture, 0);
    }
    else
    {
      glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, 0, 0);
    }
    Disable();
  }
}

#pragma region FrameBuffer
