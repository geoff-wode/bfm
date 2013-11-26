#if ! defined(__BLENDING__)
#define __BLENDING__

#include <gl_loader/gl_loader.h>

struct Blending
{
  Blending()
    : enabled(false),
      srcRGB(GL_ONE), srcAlpha(GL_ONE),
      dstRGB(GL_ZERO), dstAlpha(GL_ZERO),
      rgbEquation(GL_FUNC_ADD),
      alphaEquation(GL_FUNC_ADD)
  {
  }

  bool enabled;

  GLenum srcRGB;
  GLenum srcAlpha;

  GLenum dstRGB;
  GLenum dstAlpha;
  
  GLenum rgbEquation;
  GLenum alphaEquation;
};

#endif // __BLENDING__
