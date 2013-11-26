#if ! defined(__TEXTURE_UNIT__)
#define __TEXTURE_UNIT__

#include <boost/shared_ptr.hpp>

struct TextureUnit
{
  boost::shared_ptr<class Texture> texture;
  boost::shared_ptr<class Sampler> sampler;
};

#endif // __TEXTURE_UNIT__
