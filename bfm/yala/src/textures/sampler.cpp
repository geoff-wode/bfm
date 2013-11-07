#include <textures/sampler.h>

//------------------------------------------------------------------------

Sampler::Sampler()
{
  glGenSamplers(1, &sampler);
}

//------------------------------------------------------------------------

Sampler::~Sampler()
{
  glDeleteSamplers(1, &sampler);
}

//------------------------------------------------------------------------
void Sampler::Bind(size_t textureUnit)
{
  glBindSampler(textureUnit, sampler);
}

//------------------------------------------------------------------------
void Sampler::Unbind(size_t textureUnit)
{
  glBindSampler(textureUnit, 0);
}

//------------------------------------------------------------------------
void Sampler::SetMinFilter(GLenum value)
{
  minFilter = value;
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, value);
}

//------------------------------------------------------------------------
void Sampler::SetMagFilter(GLenum value)
{
  magFilter = value;
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, value);
}
