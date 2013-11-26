// A context contains the per-window GL state and provides rendering methods.
// A context is created by the Device object.

#if ! defined(__CORE_CONTEXT__)
#define __CORE_CONTEXT__

#include <SDL.h>
#include <vector>
#include <gl_loader/gl_loader.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <core/drawstate.h>
#include <core/clearstate.h>
#include <core/scenestate.h>

class Context : public boost::noncopyable
{
public:
  Context();
  ~Context();

  void Clear(const ClearState& clearState);
  void Draw(GLenum primitiveType, size_t vertexCount, const DrawState& drawState);
  void Draw(GLenum primitiveType, size_t vertexCount, size_t vertexStart, const DrawState& drawState);
  void DrawIndexed(GLenum primitiveType, size_t vertexCount, const DrawState& drawState);
  void DrawIndexed(GLenum primitiveType, size_t vertexCount, size_t vertexStart, const DrawState& drawState);

private:
  ClearState clearState;
  DrawState drawState;
};

typedef boost::shared_ptr<Context> ContextPtr;

#endif // __CORE_CONTEXT__
