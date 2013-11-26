#if ! defined(__VERTEX_BUFFER__)
#define __VERTEX_BUFFER__

#include <gl_loader/gl_loader.h>
#include <boost/shared_ptr.hpp>
#include <core/vertexlayout.h>

class VertexBuffer
{
public:
  VertexBuffer(const VertexLayout& vertexLayout, size_t vertexCount, GLenum usage);
  ~VertexBuffer();

  void Enable() { glBindBuffer(GL_ARRAY_BUFFER, buffer); }
  static void Disable() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  void SetData(const void* const data, size_t vertexCount, size_t startVertex = 0);

  size_t GetVertexCount() const { return vertexCount; }

  const VertexLayout& GetVertexLayout() const { return vertexLayout; }

private:
  GLuint buffer;
  const size_t vertexCount;
  const VertexLayout vertexLayout;
};

typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;

#endif // __VERTEX_BUFFER__
