#include <textures/sampler2d.h>

//------------------------------------------------------------------------

Sampler2D::Sampler2D()
{
}

//------------------------------------------------------------------------

Sampler2D::~Sampler2D()
{
}

//------------------------------------------------------------------------
void Sampler2D::SetWrapS(GLenum value)
{
  wrapS = value;
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, value);
}

//------------------------------------------------------------------------
void Sampler2D::SetWrapT(GLenum value)
{
  wrapT = value;
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, value);
}
