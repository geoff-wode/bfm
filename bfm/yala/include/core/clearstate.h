#if ! defined(__CLEAR_STATE__)
#define __CLEAR_STATE__

#include <glm/glm.hpp>

struct ClearState
{
  ClearState()
    : buffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT),
      depthBufferMask(true),
      depthValue(1.0f),
      colourMask(true, true, true, true),
      colourValue(0, 0, 0, 0)
  {
  }

  GLenum buffers;

  bool depthBufferMask;
  float depthValue;

  glm::bvec4 colourMask;
  glm::vec4 colourValue;
};

#endif // __CLEAR_STATE__
