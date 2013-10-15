#if ! defined(__DEVICE__)
#define __DEVICE__

#include <SDL.h>
#include <vector>
#include <glm/glm.hpp>
#include <gl_loader/gl_loader.h>
#include <boost/noncopyable.hpp>
#include <effect.h>
#include <vertexarray.h>
#include <texture2d.h>
#include <boost/shared_ptr.hpp>

//----------------------------------------------------------

struct ClearState
{
  ClearState()
    : depthBufferMask(true),
      depthValue(1.0f),
      colourMask(true, true, true, true),
      colourValue(0, 0, 0, 0)
  {
  }

  bool depthBufferMask;
  float depthValue;

  glm::bvec4 colourMask;
  glm::vec4 colourValue;
};

//----------------------------------------------------------
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

struct DrawState
{
  DrawState()
    : depthBufferMask(true),
      colourMask(true, true, true, true),
      polygonMode(GL_FILL)
  {
  }

  bool depthBufferMask;
  glm::bvec4 colourMask;

  GLenum polygonMode;

  Culling culling;
  DepthTest depthTest;
};

//----------------------------------------------------------
struct RenderState
{
  RenderState()
    : effect(NULL), vertexArray(NULL)
  {
    std::memset(textureUnits, 0, sizeof(textureUnits));
  }

  Effect* effect;
  VertexArray* vertexArray;

  static const size_t MaxTextures = 16;
  Texture2D* textureUnits[MaxTextures];

  DrawState drawState;
};

//----------------------------------------------------------
class Device : public boost::noncopyable
{
public:
  Device();
  ~Device();

  bool Initialise(size_t backbufferWidth, size_t backbufferHeight, const char* const windowTitle);

  void Clear(GLenum buffers, const ClearState& clearState);

  void Draw(GLenum primitiveType, size_t start, size_t count, const RenderState& renderState);

  void SwapBuffers();

  const size_t& BackbufferWidth;
  const size_t& BackbufferHeight;

private:
  size_t backbufferWidth;
  size_t backbufferHeight;

  SDL_Window* mainWindow;
  SDL_GLContext glContext;

  ClearState clearState;
  RenderState renderState;

  void ApplyClearState(const ClearState& state);
  void ApplyRenderState(const RenderState& state);
  void ApplyColourMask(const glm::bvec4& mask);
  void ApplyDepthMask(bool mask);
  void ApplyCulling(const Culling& state);
  void ApplyDepthTest(const DepthTest& state);
  void ApplyTextureUnits(Texture2D* const textures[]);
  void ApplyEffect(Effect* effect);
  void ApplyVertexArray(VertexArray* vertexArray);
  void ApplyPolygonMode(GLenum mode);
};

#endif // __DEVICE__
