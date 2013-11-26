#include <logging.h>
#include <textures/texture2d.h>
#include <device.h>
#include <SOIL.h>
#include <utils.h>

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
void Texture2D::Load(const void* const data, GLenum dataFormat, GLenum dataType, const Texture2DDescription& description)
{
  // Make sure none of the texture slots bound to the device are modified. (Gotta love global state!)
  MakeActive(RenderState::MaxTextures);
  Bind();
  CALLONEXIT(glBindTexture, texture, 0);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    textureFormat,
    description.size.x,
    description.size.y,
    0,
    dataFormat,
    dataType,
    data);

  this->dataFormat = dataFormat;
  this->dataType = dataType;
  this->description = description;
}

//------------------------------------------------------------------------
void Texture2D::Load(const char* const filename, GLenum dataFormat, GLenum dataType, const Texture2DDescription& description)
{
  const unsigned int flags = SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS;

  const unsigned int channels = (dataFormat == GL_ALPHA) ? SOIL_LOAD_L : SOIL_LOAD_AUTO;

  MakeActive(RenderState::MaxTextures);
  Bind();
  CALLONEXIT(glBindTexture, texture, 0);

  if (SOIL_load_OGL_texture(filename, channels, texture, flags))
  {
    this->dataFormat = dataFormat;
    this->dataType = dataType;
    this->description = description;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&description.size.x);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&description.size.y);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&description.internalFormat);
  }
  else
  {
    LOG("error loading texture %s: %s\n", filename, SOIL_last_result());
  }
}
