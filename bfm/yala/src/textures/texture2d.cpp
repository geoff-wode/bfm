#include <logging.h>
#include <textures/texture2d.h>
#include <device.h>
#include <SOIL.h>

//------------------------------------------------------------------------

Texture2D::Texture2D()
  : Texture(GL_TEXTURE_2D)
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
  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
  Bind();

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

  Unbind();
}

//------------------------------------------------------------------------
void Texture2D::Load(const char* const filename, GLenum dataFormat, GLenum dataType, GLenum textureFormat)
{
  const unsigned int flags = SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS;

  const unsigned int channels = (dataFormat == GL_ALPHA) ? SOIL_LOAD_L : SOIL_LOAD_AUTO;

  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
  Bind();

  if (SOIL_load_OGL_texture(filename, channels, texture, flags))
  {
    this->dataFormat = dataFormat;
    this->dataType = dataType;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&height);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&textureFormat);
  }
  else
  {
    LOG("error loading texture %s: %s\n", filename, SOIL_last_result());
  }

  Unbind();
}
