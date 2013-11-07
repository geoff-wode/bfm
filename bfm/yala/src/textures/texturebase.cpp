
#include <device.h>
#include <textures/texturebase.h>

//------------------------------------------------------------------------

Texture::Texture(GLenum type, boost::shared_ptr<Sampler> sampler)
  : type(type),
    sampler(sampler),
    baseLOD(0), maxLOD(0)
{
  glGenTextures(1, &texture);
}

//------------------------------------------------------------------------
Texture::~Texture()
{
  glDeleteTextures(1, &texture);
}

//------------------------------------------------------------------------
void Texture::BindToTextureUnit(size_t textureUnit)
{
  glBindTexture(type, texture);
  sampler->Bind(textureUnit);
}

//------------------------------------------------------------------------
void Texture::Activate(size_t textureUnit)
{
  glActiveTexture(GL_TEXTURE0 + textureUnit);
}

//------------------------------------------------------------------------
void Texture::AssignSampler(boost::shared_ptr<Sampler> sampler)
{
  this->sampler = sampler;
}
