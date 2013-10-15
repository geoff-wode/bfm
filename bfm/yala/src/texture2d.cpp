#include <texture2d.h>
#include <device.h>

//------------------------------------------------------------------------
Texture2D::Texture2D()
{
  glGenTextures(1, &texture);
  glGenSamplers(1, &sampler);
}

//------------------------------------------------------------------------
Texture2D::~Texture2D()
{
  glDeleteTextures(1, &texture);
  glDeleteSamplers(1, &sampler);
}
