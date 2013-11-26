#if ! defined(__DEPTH_TEST__)
#define __DEPTH_TEST__

#include <gl_loader/gl_loader.h>

struct DepthTest
{
  DepthTest()
    : enabled(true),
      function(GL_LEQUAL)
  {
  }

  bool enabled;
  GLenum function;
};

#endif // __DEPTH_TEST__
