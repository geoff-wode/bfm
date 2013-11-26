
#include <core/context.h>

//------------------------------------------------------------------------

static void ForceClearState(const ClearState& state);
static void ForceDrawState(const DrawState& state);

static void ApplyClearState(const ClearState& newState, ClearState& oldState);
static void ApplyDrawState(const DrawState& newState, DrawState& oldState);

static void ApplyEffect(Effect* const effect, DrawState& oldState);
static void ApplyVertexArray(VertexArrayPtr vertexArray, DrawState& oldState);
static void ApplyRenderState(const RenderState& newState, RenderState& oldState);
static void ApplyColourMask(const glm::bvec4& newState, glm::bvec4& oldState);
static void ApplyDepthMask(bool newState, bool& oldState);
static void ApplyCulling(const Culling& newState, Culling& oldState);
static void ApplyPolygonMode(GLenum newMode, GLenum& oldMode);

//------------------------------------------------------------------------

Context::Context()
{
  ForceClearState(clearState);
  ForceDrawState(drawState);
}

//------------------------------------------------------------------------

Context::~Context()
{
}

//------------------------------------------------------------------------

void Context::Clear(const ClearState& clearState)
{
  ApplyClearState(clearState, this->clearState);
  glClear(clearState.buffers);
}

//------------------------------------------------------------------------

void Context::Draw(GLenum primitiveType, size_t vertexCount, const DrawState& drawState)
{
  Draw(primitiveType, vertexCount, 0, drawState);
}

//------------------------------------------------------------------------

void Context::Draw(GLenum primitiveType, size_t vertexCount, size_t vertexStart, const DrawState& drawState)
{
  ApplyDrawState(drawState, this->drawState);

  glDrawArrays(primitiveType, vertexStart, vertexCount);
}

//------------------------------------------------------------------------

void Context::DrawIndexed(GLenum primitiveType, size_t vertexCount, const DrawState& drawState)
{
  DrawIndexed(primitiveType, vertexCount, 0, drawState);
}

//------------------------------------------------------------------------

void Context::DrawIndexed(GLenum primitiveType, size_t vertexCount, size_t vertexStart, const DrawState& drawState)
{
  ApplyDrawState(drawState, this->drawState);

  const GLenum indexType = drawState.vertexArray->GetIndexBuffer()->GetIndexType();
  glDrawElements(primitiveType, vertexCount, indexType, (const void*)vertexStart);
}

//------------------------------------------------------------------------

static void ForceClearState(const ClearState& state)
{
  glClearColor(state.colourValue.r, state.colourValue.g, state.colourValue.b, state.colourValue.a);
  glClearDepth(state.depthValue);
}

//------------------------------------------------------------------------

static void ForceDrawState(const DrawState& state)
{
  glColorMask(
    state.renderState.colourMask.r,
    state.renderState.colourMask.g,
    state.renderState.colourMask.b,
    state.renderState.colourMask.a);
  
  glDepthMask(state.renderState.depthMask);

  state.renderState.depthTest.enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
  glDepthFunc(state.renderState.depthTest.function);

  state.renderState.culling.enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  glCullFace(state.renderState.culling.faceToCull);
  glFrontFace(state.renderState.culling.windingOrder);

  glPolygonMode(GL_FRONT_AND_BACK, state.renderState.mode);

  state.renderState.blending.enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);

  glBlendFuncSeparate(
    state.renderState.blending.srcRGB, state.renderState.blending.dstRGB,
    state.renderState.blending.srcAlpha, state.renderState.blending.dstAlpha);
  glBlendEquationSeparate(state.renderState.blending.rgbEquation, state.renderState.blending.alphaEquation);
}

//------------------------------------------------------------------------

static void ApplyClearState(const ClearState& newState, ClearState& oldState)
{
  if (newState.colourValue != oldState.colourValue)
  {
    glClearColor(newState.colourValue.r, newState.colourValue.g, newState.colourValue.b, newState.colourValue.a);
    oldState.colourValue = newState.colourValue;
  }

  if (newState.depthValue != oldState.depthValue)
  {
    glClearDepth(newState.depthValue);
    oldState.depthValue = newState.depthValue;
  }

  ApplyColourMask(newState.colourMask, oldState.colourMask);
  ApplyDepthMask(newState.depthBufferMask, oldState.depthBufferMask);
}

//------------------------------------------------------------------------

static void ApplyDrawState(const DrawState& newState, DrawState& oldState)
{
  ApplyEffect(newState.effect, oldState);
  ApplyVertexArray(newState.vertexArray, oldState);
  ApplyRenderState(newState.renderState, oldState.renderState);
}

static void ApplyRenderState(const RenderState& newState, RenderState& oldState)
{
  ApplyColourMask(newState.colourMask, oldState.colourMask);
  ApplyDepthMask(newState.depthMask, oldState.depthMask);
}

//------------------------------------------------------------------------

static void ApplyColourMask(const glm::bvec4& newState, glm::bvec4& oldState)
{
  if (newState != oldState)
  {
    glColorMask(newState.r, newState.g, newState.b, newState.a);
    oldState = newState;
  }
}

//------------------------------------------------------------------------

static void ApplyDepthMask(bool newState, bool& oldState)
{
  if (newState != oldState)
  {
    glDepthMask(newState);
    oldState = newState;
  }
}

//------------------------------------------------------------------------

static void ApplyCulling(const Culling& newState, Culling& oldState)
{
  if (oldState.enabled != newState.enabled)
  {
    oldState.enabled = newState.enabled;
    if (oldState.enabled)
    {
      glEnable(GL_CULL_FACE);
    }
    else
    {
      glDisable(GL_CULL_FACE);
    }
  }
  if (oldState.faceToCull != newState.faceToCull)
  {
    oldState.faceToCull = newState.faceToCull;
    glCullFace(newState.faceToCull);
  }
  if (oldState.windingOrder != newState.windingOrder)
  {
    oldState.windingOrder = newState.windingOrder;
    glFrontFace(newState.windingOrder);
  }
}

//------------------------------------------------------------------------

static void ApplyPolygonMode(GLenum newMode, GLenum& oldMode)
{
  if (newMode != oldMode)
  {
    oldMode = newMode;
    glPolygonMode(GL_FRONT_AND_BACK, newMode);
  }
}

//------------------------------------------------------------------------

static void ApplyEffect(Effect* const effect, DrawState& oldState)
{
  if (effect != oldState.effect)
  {
    oldState.effect = effect;
    effect->Enable();
  }
}

//------------------------------------------------------------------------

static void ApplyVertexArray(VertexArrayPtr vertexArray, DrawState& oldState)
{
  if (vertexArray != oldState.vertexArray)
  {
    oldState.vertexArray = vertexArray;
    vertexArray->Enable();
  }
}
