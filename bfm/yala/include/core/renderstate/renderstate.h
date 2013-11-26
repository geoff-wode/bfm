#if ! defined(__RENDER_STATE__)
#define __RENDER_STATE__

#include <core/renderstate/blending.h>
#include <core/renderstate/culling.h>
#include <core/renderstate/depthtest.h>

struct RenderState
{
  RenderState()
    : mode(GL_FILL),
      depthMask(true),
      colourMask(true, true, true, true)
  { }

  GLenum mode;        // point/line/fill
  bool depthMask;
  glm::bvec4 colourMask;
  Blending blending;
  Culling culling;
  DepthTest depthTest;
};

#endif // __RENDER_STATE__
