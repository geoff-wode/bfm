#include <textures/texture2d.h>
#include <device.h>

//------------------------------------------------------------------------

Texture2D::Texture2D()
  : Texture(GL_TEXTURE_2D, NULL)
{
}

Texture2D::Texture2D(boost::shared_ptr<Sampler> sampler)
  : Texture(GL_TEXTURE_2D, sampler)
{
}

//------------------------------------------------------------------------
Texture2D::~Texture2D()
{
}

//------------------------------------------------------------------------
void Texture2D::Load(const void* const data, size_t width, size_t height, GLenum dataFormat, GLenum dataType, GLenum textureFormat)
{
  // Make sure none of the texture slots bound to the device are modified. (Gotta love global state!)
  Activate(RenderState::MaxTextures);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, baseLOD);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLOD);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    textureFormat,
    width,
    height,
    0,
    dataFormat,
    dataType,
    data);

  this->width = width;
  this->height = height;
  this->dataFormat = dataFormat;
  this->dataType = dataType;
  this->textureFormat = textureFormat;
}

//------------------------------------------------------------------------
void Texture2D::UnbindTextureUnit(size_t textureUnit)
{
  glBindTexture(GL_TEXTURE_2D, 0);
  Sampler::Unbind(textureUnit);
}
