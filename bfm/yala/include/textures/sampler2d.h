// Texture sampler object.

#if ! defined(__SAMPLER_2D__)
#define __SAMPLER_2D__

#include <textures/sampler.h>
#include <gl_loader/gl_loader.h>

class Sampler2D : public Sampler
{
public:
  Sampler2D();
  virtual ~Sampler2D();

  void SetWrapS(GLenum value);
  void SetWrapT(GLenum value);

  GLenum GetWrapS() const { return wrapS; }
  GLenum GetWrapT() const { return wrapT; }

protected:
  GLenum wrapS;
  GLenum wrapT;
};


#endif // __SAMPLER_2D__
