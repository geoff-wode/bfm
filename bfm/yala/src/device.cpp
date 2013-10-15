#include <logging.h>
#include <device.h>
#include <glm/ext.hpp>
#include <boost/make_shared.hpp>

//----------------------------------------------------------

struct SDLAttribute
{
  SDL_GLattr  attr;
  int         value;
  const char* name;
};
#define SDL_ATTR(name, value) { name, value, #name }
static const SDLAttribute sdlAttributes[] =
{
  SDL_ATTR(SDL_GL_DOUBLEBUFFER,       1 ),
  SDL_ATTR(SDL_GL_RED_SIZE,           8 ),
  SDL_ATTR(SDL_GL_GREEN_SIZE,         8 ),
  SDL_ATTR(SDL_GL_BLUE_SIZE,          8 ),
  SDL_ATTR(SDL_GL_ALPHA_SIZE,         8 ),
  SDL_ATTR(SDL_GL_DEPTH_SIZE,         24),
  SDL_ATTR(SDL_GL_STENCIL_SIZE,       8 ),
  SDL_ATTR(SDL_GL_BUFFER_SIZE,        24),
  SDL_ATTR(SDL_GL_MULTISAMPLEBUFFERS, 1 ),
  SDL_ATTR(SDL_GL_MULTISAMPLESAMPLES, 2 )
};
const int NumSDLAttributes = sizeof(sdlAttributes)/sizeof(sdlAttributes[0]);

//----------------------------------------------------------

static void ConfigureSDL();
static void ReportSDLConfig();
static void ConfigureOpenGL(const ClearState& clearState, const DrawState& drawState);

//----------------------------------------------------------
Device::Device()
  : BackbufferWidth(backbufferWidth), BackbufferHeight(backbufferHeight)
{
}

//----------------------------------------------------------
Device::~Device()
{
  if (glContext) { SDL_GL_DeleteContext(glContext); }
  if (mainWindow) { SDL_DestroyWindow(mainWindow); }
  SDL_Quit();
}

//----------------------------------------------------------
bool Device::Initialise(size_t backbufferWidth, size_t backbufferHeight, const char* const windowTitle)
{
  bool isInitialised = false;

  this->backbufferWidth = backbufferWidth;
  this->backbufferHeight = backbufferHeight;

  SDL_Init(SDL_INIT_EVERYTHING);

  ConfigureSDL();
  
  mainWindow = SDL_CreateWindow(
    windowTitle,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    BackbufferWidth, BackbufferHeight,
    SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);

  if (mainWindow)
  {
    glContext = SDL_GL_CreateContext(mainWindow);
    if (glContext)
    {
      SDL_ShowWindow(mainWindow);
      isInitialised = true;
    }
  }

  if (isInitialised)
  {
    ConfigureSDL();
    ConfigureOpenGL(clearState, renderState.drawState);
    ReportSDLConfig();

    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    LOG("max. texture units: %d\n", maxTextureUnits);
    ASSERT(RenderState::MaxTextures <= maxTextureUnits);
  }

  SDL_SetRelativeMouseMode(SDL_TRUE);

  return isInitialised;
}

//----------------------------------------------------------
void Device::Clear(GLenum buffers, const ClearState& clearState)
{
  ApplyClearState(clearState);
  glClear(buffers);
}

//----------------------------------------------------------
void Device::Draw(GLenum primitiveType, size_t start, size_t count, const RenderState& renderState)
{
  ApplyRenderState(renderState);

  renderState.effect->Appply();

  const boost::shared_ptr<IndexBuffer> indexBuffer = renderState.vertexArray->GetIndexBuffer();
  if (indexBuffer)
  {
    glDrawElements(primitiveType, count, indexBuffer->GetIndexType(), (const void*)start);
  }
  else
  {
    glDrawArrays(primitiveType, start, count);
  }
}

//----------------------------------------------------------
void Device::SwapBuffers()
{
  SDL_GL_SwapWindow(mainWindow);
}

//----------------------------------------------------------
static void ConfigureSDL()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  for (int i = 0; i < NumSDLAttributes; ++i)
  {
    SDL_GL_SetAttribute(sdlAttributes[i].attr, sdlAttributes[i].value);
  }
}

//-----------------------------------------------------------
static void ReportSDLConfig()
{
  for (int i = 0; i < NumSDLAttributes; ++i)
  {
    int value;
    SDL_GL_GetAttribute(sdlAttributes[i].attr, &value);
    LOG("%s = %d (requested %d)\n", sdlAttributes[i].name, value, sdlAttributes[i].value);
  }
}

//-----------------------------------------------------------
void Device::ApplyClearState(const ClearState& state)
{
  ApplyColourMask(state.colourMask);
  ApplyDepthMask(state.depthBufferMask);

  if (state.colourValue != clearState.colourValue)
  {
    clearState.colourValue = state.colourValue;
    glClearColor(clearState.colourValue.r, clearState.colourValue.g, clearState.colourValue.b, clearState.colourValue.a);
  }

  if (state.depthValue != clearState.depthValue)
  {
    clearState.depthValue = state.depthValue;
    glClearDepth(clearState.depthValue);
  }
}
//-----------------------------------------------------------
void Device::ApplyRenderState(const RenderState& state)
{
  ApplyColourMask(state.drawState.colourMask);
  ApplyDepthMask(state.drawState.depthBufferMask);
  ApplyCulling(state.drawState.culling);
  ApplyDepthTest(state.drawState.depthTest);
  ApplyEffect(state.effect);
  ApplyVertexArray(state.vertexArray);
  ApplyTextureUnits(state.textureUnits);
  ApplyPolygonMode(state.drawState.polygonMode);
}
//-----------------------------------------------------------
void Device::ApplyColourMask(const glm::bvec4& mask)
{
  if (mask != renderState.drawState.colourMask)
  {
    renderState.drawState.colourMask = mask;
    glColorMask(mask.r, mask.g, mask.b, mask.a);
  }
}
//-----------------------------------------------------------
void Device::ApplyDepthMask(bool mask)
{
  if (mask != renderState.drawState.depthBufferMask)
  {
    renderState.drawState.depthBufferMask = mask;
    glDepthMask(mask);
  }
}
//-----------------------------------------------------------
void Device::ApplyCulling(const Culling& state)
{
  if (renderState.drawState.culling.enabled != state.enabled)
  {
    renderState.drawState.culling.enabled = state.enabled;
    if (renderState.drawState.culling.enabled)
    {
      glEnable(GL_CULL_FACE);
    }
    else
    {
      glDisable(GL_CULL_FACE);
    }
  }
  if (renderState.drawState.culling.faceToCull != state.faceToCull)
  {
    renderState.drawState.culling.faceToCull = state.faceToCull;
    glCullFace(state.faceToCull);
  }
  if (renderState.drawState.culling.windingOrder != state.windingOrder)
  {
    renderState.drawState.culling.windingOrder = state.windingOrder;
    glFrontFace(state.windingOrder);
  }
}
//-----------------------------------------------------------
void Device::ApplyDepthTest(const DepthTest& state)
{
  if (state.enabled != renderState.drawState.depthTest.enabled)
  {
    renderState.drawState.depthTest.enabled = state.enabled;
    if (state.enabled)
    {
      glEnable(GL_DEPTH_TEST);
      if (state.function != renderState.drawState.depthTest.function)
      {
        renderState.drawState.depthTest.function = state.function;
        glDepthFunc(state.function);
      }
    }
    else
    {
      glDisable(GL_DEPTH_TEST);
    }
  }
}
//-----------------------------------------------------------
void Device::ApplyTextureUnits(Texture2D* const textures[])
{
  for (size_t i = 0; i < RenderState::MaxTextures; ++i)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    if (textures[i] != renderState.textureUnits[i])
    {
      renderState.textureUnits[i] = textures[i];
      if (renderState.textureUnits[i])
      {
        glBindTexture(GL_TEXTURE_2D, renderState.textureUnits[i]->texture);
        glBindSampler(i, renderState.textureUnits[i]->sampler);
      }
      else
      {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindSampler(i, 0);
      }
    }
  }
  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
}
//-----------------------------------------------------------
void Device::ApplyEffect(Effect* effect)
{
  if (effect != renderState.effect)
  {
    renderState.effect = effect;
    effect->Enable();
  }
}
//-----------------------------------------------------------
void Device::ApplyVertexArray(VertexArray* vertexArray)
{
  if (vertexArray != renderState.vertexArray)
  {
    renderState.vertexArray = vertexArray;
    vertexArray->Enable();
  }
}
//-----------------------------------------------------------
void Device::ApplyPolygonMode(GLenum mode)
{
  if (mode != renderState.drawState.polygonMode)
  {
    renderState.drawState.polygonMode = mode;
    glPolygonMode(GL_FRONT_AND_BACK, mode);
  }
}
//-----------------------------------------------------------
static void ConfigureOpenGL(const ClearState& clearState, const DrawState& drawState)
{
  glInitLibrary();
  GLint majorVersion;
  GLint minorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  LOG("GL version: %d.%d\n", majorVersion, minorVersion);
  LOG("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  LOG("Vendor: %s\n", glGetString(GL_VENDOR));
  LOG("Renderer: %s\n", glGetString(GL_RENDERER));

  glClearColor(clearState.colourValue.r, clearState.colourValue.g, clearState.colourValue.b, clearState.colourValue.a);
  glClearDepth(clearState.depthValue);

  glColorMask(drawState.colourMask.r, drawState.colourMask.g, drawState.colourMask.b, drawState.colourMask.a);
  glDepthMask(drawState.depthBufferMask);

  glCullFace(drawState.culling.faceToCull);
  glFrontFace(drawState.culling.windingOrder);
  drawState.culling.enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

  glDepthFunc(drawState.depthTest.function);
  drawState.depthTest.enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);

  glPolygonMode(GL_FRONT_AND_BACK, drawState.polygonMode);
}
