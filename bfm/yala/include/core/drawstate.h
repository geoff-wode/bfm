#if ! defined(__DRAW_STATE__)
#define __DRAW_STATE__

#include <core/vertexarray.h>
#include <core/effect/effect.h>
#include <core/renderstate/renderstate.h>

struct DrawState
{
  Effect*         effect;
  RenderState     renderState;
  VertexArrayPtr  vertexArray;
};

#endif // __DRAW_STATE__
