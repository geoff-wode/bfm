
#if ! defined(__TEXTURE_BASE_H__)
#define __TEXTURE_BASE_H__

#include <cstddef>
#include <textures/sampler.h>
#include <boost/shared_ptr.hpp>
#include <gl_loader/gl_loader.h>


/// Base class for textures.
class Texture
{
public:
  virtual ~Texture();

  void AssignSampler(boost::shared_ptr<Sampler> sampler);

  GLenum GetDataFormat()    const { return dataFormat; }
  GLenum GetDataType()      const { return dataType; }
  GLenum GetTextureFormat() const { return textureFormat; }

  void SetMinFilter(GLenum value);
  void SetMagFilter(GLenum value);

  virtual void Activate(size_t textureUnit);
  virtual void BindToTextureUnit(size_t textureUnit);

//protected:
  Texture(GLenum type);
  Texture(GLenum type, boost::shared_ptr<Sampler> sampler);

  const GLenum type;

  boost::shared_ptr<Sampler> sampler;

  GLuint texture;
  size_t baseLOD;
  size_t maxLOD;
  GLenum dataFormat;
  GLenum dataType;
  GLenum textureFormat;
};


#endif // __TEXTURE_BASE_H__
