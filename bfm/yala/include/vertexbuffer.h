#if ! defined(__VERTEX_BUFFER__)
#define __VERTEX_BUFFER__

#include <gl_loader/gl_loader.h>
#include <boost/shared_ptr.hpp>
#include <vertexlayout.h>

class VertexBuffer
{
public:
  VertexBuffer(const VertexLayout& vertexLayout, size_t vertexCount, GLenum usage, const void* const data = NULL);
  ~VertexBuffer();

  void Enable() { glBindBuffer(GL_ARRAY_BUFFER, buffer); }
  void Disable() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  void SetData(const void* const data, size_t vertexCount, size_t startVertex = 0);

  size_t GetVertexCount() const { return vertexCount; }

  const VertexLayout& GetVertexLayout() const { return vertexLayout; }

private:
  size_t vertexCount;
  GLuint buffer;
  const VertexLayout vertexLayout;
};

#endif // __VERTEX_BUFFER__
