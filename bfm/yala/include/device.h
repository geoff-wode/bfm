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
      colourMask(true, true, true, true)
  {
  }

  bool depthBufferMask;
  glm::bvec4 colourMask;

  Culling culling;
  DepthTest depthTest;
};

//----------------------------------------------------------
struct RenderState
{
  Effect* effect;
  VertexArray* vertexArray;

  static const size_t MaxTextures = 16;
  boost::shared_ptr<Texture2D> textureUnits[MaxTextures];

  DrawState drawState;
};

//----------------------------------------------------------
class Device : public boost::noncopyable
{
public:
  Device();
  ~Device();

  bool Initialise(const char* const windowTitle);

  void Clear(GLenum buffers, const ClearState& clearState);
  void Draw(GLenum primitiveType, size_t start, size_t count, const RenderState& renderState);
  void SwapBuffers();

  size_t BackbufferWidth;
  size_t BackbufferHeight;

private:
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
  void ApplyTextureUnits(const boost::shared_ptr<Texture2D> textures[]);
  void ApplyEffect(Effect* effect);
  void ApplyVertexArray(VertexArray* vertexArray);
};

#endif // __DEVICE__
