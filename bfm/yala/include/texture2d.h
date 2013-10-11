#if ! defined(__TEXTURE2D__)
#define __TEXTURE2D__

#include <boost/shared_ptr.hpp>
#include <gl_loader/gl_loader.h>

struct Texture2D
{
  Texture2D();
  ~Texture2D();

  size_t width;
  size_t height;

  GLuint texture;
  GLuint sampler;
};

boost::shared_ptr<Texture2D> LoadHeightMap(const void* const data, size_t width, size_t height);

#endif // __TEXTURE2D__
