#if ! defined(__CULLING__)
#define __CULLING__

#include <gl_loader/gl_loader.h>

struct Culling
{
  Culling()
    : enabled(true),
      faceToCull(GL_BACK),
      windingOrder(GL_CCW)
  {
  }

  bool enabled;
  GLenum faceToCull;
  GLenum windingOrder;
};

#endif // __CULLING__
