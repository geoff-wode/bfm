#if ! defined(__TEXTURE_UNIT__)
#define __TEXTURE_UNIT__

#include <boost/shared_ptr.hpp>
#include <textures/sampler.h>
#include <textures/texture.h>

class TextureUnit
{
public:
  TextureUnit(size_t slot);
  ~TextureUnit();

  void SetTexture(boost::shared_ptr<Texture> texture) { this->texture = texture; }
  void SetSampler(boost::shared_ptr<Sampler> sampler) { this->sampler = sampler; }

  boost::shared_ptr<Texture> GetTexture() { return texture; }
  boost::shared_ptr<Sampler> GetSampler() { return sampler; }

  void Enable();
  void Disable();

private:
  const size_t slot;

  boost::shared_ptr<Texture> texture;
  boost::shared_ptr<Sampler> sampler;
};

#endif // __TEXTURE_UNIT__
