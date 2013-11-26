#if ! defined(__VERTEX_ARRAY__)
#define __VERTEX_ARRAY__

#include <core/vertexlayout.h>
#include <core/buffers//vertexbuffer.h>
#include <core/buffers/indexbuffer.h>
#include <gl_loader/gl_loader.h>
#include <boost/shared_ptr.hpp>

class VertexArray
{
public:
  VertexArray(VertexBufferPtr vertexBuffer);
  VertexArray(VertexBufferPtr vertexBuffer, IndexBufferPtr indexBuffer);

  ~VertexArray();

  void Enable() { glBindVertexArray(vao); }
  static void Disable() { glBindVertexArray(0); }

  VertexBufferPtr GetVertexBuffer() const { return vertexBuffer; }
  IndexBufferPtr GetIndexBuffer() const { return indexBuffer; }

  const VertexLayout& GetVertexLayout() const { return vertexBuffer->GetVertexLayout(); }

private:
  GLuint vao;
  VertexBufferPtr vertexBuffer;
  IndexBufferPtr indexBuffer;

  void Initialise(VertexBufferPtr vertexBuffer, IndexBufferPtr indexBuffer);
};

typedef boost::shared_ptr<VertexArray> VertexArrayPtr;

#endif // __VERTEX_ARRAY__
