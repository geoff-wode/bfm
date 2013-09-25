#if ! defined(__TEXTURE2D__)
#define __TEXTURE2D__

#include <cstddef>
#include <gl_loader/gl_loader.h>

struct Texture2D
{
  static const size_t MaxTextureUnit = 31;
  GLuint handle;
  GLuint sampler;

  Texture2D()
  {
    glGenTextures(1, &handle);
    glGenSamplers(1, &sampler);
  }
  
  ~Texture2D()
  {
    glDeleteSamplers(1, &sampler);
    glDeleteTextures(1, &handle);
  }

  void Bind(size_t textureUnit)
  {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindSampler(textureUnit, sampler);
    glBindTexture(GL_TEXTURE_2D, handle);
  }

  void Unbind(size_t textureUnit)
  {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindSampler(textureUnit, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE31);
  }
};

#endif // __TEXTURE2D__
