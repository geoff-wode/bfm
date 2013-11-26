#if ! defined(__TEXTURE2D__)
#define __TEXTURE2D__

#include <glm/glm.hpp>
#include <textures/texture.h>
#include <boost/shared_ptr.hpp>
#include <gl_loader/gl_loader.h>

struct Texture2DDescription
{
  Texture2DDescription() { }

  Texture2DDescription(GLenum internalFormat, const glm::uvec2& size, bool makeMipMaps = false)
    : makeMipMaps(makeMipMaps),
      internalFormat(internalFormat),
      size(size)
  {
  }

  bool makeMipMaps;
  GLenum internalFormat;
  glm::uvec2 size;
};

class Texture2D : public Texture
{
public:
  Texture2D();

  virtual ~Texture2D();

  // Load a texture image from memory.
  // @param data [in]
  // @param width [in] width of input data in pixels
  // @param height [in] height of input data in pixels
  // @param dataFormat [in] format of the input data
  // @param dataType [in] type of the input data (e.g. GL_FLOAT, GL_BYTE, etc.)
  // @param textureFormat [in] internal format to use for texture data
  void Load(const void* const data, GLenum dataFormat, GLenum dataType, const Texture2DDescription& description);

  void Load(const char* const filename, GLenum dataFormat, GLenum dataType, const Texture2DDescription& description);

  unsigned int GetWidth() const { return description.size.x; }
  unsigned int GetHeight() const { return description.size.y; }

  virtual void Bind() { glBindTexture(GL_TEXTURE_2D, texture); }
  virtual void Unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

private:
  Texture2DDescription description;
};

#endif // __TEXTURE2D__
