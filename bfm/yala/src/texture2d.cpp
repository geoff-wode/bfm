#include <texture2d.h>
#include <device.h>

//------------------------------------------------------------------------
Texture2D::Texture2D()
{
}

//------------------------------------------------------------------------
Texture2D::~Texture2D()
{
  glDeleteTextures(1, &texture);
  glDeleteSamplers(1, &sampler);
}

//------------------------------------------------------------------------
void Texture2D::LoadHeightMap(const void* const data, size_t width, size_t height)
{
  this->width = width;
  this->height = height;

  glGenTextures(1, &texture);
  glGenSamplers(1, &sampler);

  // Ensure that loading the texture does not interfere with any existing sampler state...
  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);

  // Bind and allocate the texture...
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Configure the sampler state appropriate for height maps;
  //  - no mirroring
  //  - no mipmapping
  //  - no filtering
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
