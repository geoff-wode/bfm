#if ! defined(__TEXTURE2D__)
#define __TEXTURE2D__

#include <textures/texturebase.h>
#include <boost/shared_ptr.hpp>
#include <gl_loader/gl_loader.h>


class Texture2D : public Texture
{
public:
  Texture2D();
  Texture2D(boost::shared_ptr<Sampler> sampler);

  virtual ~Texture2D();

  // Load a texture image from memory.
  // @param data [in]
  // @param width [in] width of input data in pixels
  // @param height [in] height of input data in pixels
  // @param dataFormat [in] format of the input data
  // @param dataType [in] type of the input data (e.g. GL_FLOAT, GL_BYTE, etc.)
  // @param textureFormat [in] internal format to use for texture data
  void Load(const void* const data, size_t width, size_t height, GLenum dataFormat, GLenum dataType, GLenum textureFormat);

  size_t GetWidth() const { return width; }
  size_t GetHeight() const { return height; }

  static void UnbindTextureUnit(size_t textureUnit);

private:
  size_t width;
  size_t height;
};

#endif // __TEXTURE2D__
