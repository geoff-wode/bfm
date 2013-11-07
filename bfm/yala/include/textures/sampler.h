// Texture sampler object.

#if ! defined(__SAMPLER__)
#define __SAMPLER__

#include <gl_loader/gl_loader.h>

class Sampler
{
public:
  virtual ~Sampler();

  void Bind(size_t textureUnit);
  static void Unbind(size_t textureUnit);

  void SetMinFilter(GLenum value);
  void SetMagFilter(GLenum value);

  GLenum GetMinFilter() const { return minFilter; }
  GLenum GetMagFilter() const { return magFilter; }

protected:
  Sampler();

  GLuint sampler;
  GLenum minFilter;
  GLenum magFilter;
};

#endif // __SAMPLER__
