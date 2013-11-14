
#if ! defined(__TEXTURE_BASE_H__)
#define __TEXTURE_BASE_H__

#include <boost/shared_ptr.hpp>
#include <gl_loader/gl_loader.h>

/// Base class for textures.
class Texture
{
public:
  virtual ~Texture()
  {
    glDeleteTextures(1, &texture);
  }

  GLenum GetDataFormat()    const { return dataFormat; }
  GLenum GetDataType()      const { return dataType; }
  GLenum GetTextureFormat() const { return textureFormat; }

  virtual void Bind() = 0;
  virtual void Unbind() = 0;

protected:
  Texture(GLenum type) : type(type)
  {
    glGenTextures(1, &texture);
  }

  GLuint texture;
  const GLenum type;
  GLenum dataFormat;
  GLenum dataType;
  GLenum textureFormat;
};

#endif // __TEXTURE_BASE_H__
