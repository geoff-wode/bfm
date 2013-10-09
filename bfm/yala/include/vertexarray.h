#if ! defined(__VERTEX_ARRAY__)
#define __VERTEX_ARRAY__

#include <vertexlayout.h>
#include <vertexbuffer.h>
#include <indexbuffer.h>
#include <gl_loader/gl_loader.h>
#include <boost/shared_ptr.hpp>

class VertexArray
{
public:
  VertexArray(boost::shared_ptr<VertexBuffer> vertexBuffer);
  VertexArray(boost::shared_ptr<VertexBuffer> vertexBuffer, boost::shared_ptr<IndexBuffer> indexBuffer);
  ~VertexArray();

  void Enable() { glBindVertexArray(vao); }

  boost::shared_ptr<VertexBuffer> GetVertexBuffer() const { return vertexBuffer; }
  boost::shared_ptr<IndexBuffer> GetIndexBuffer() const { return indexBuffer; }

  const VertexLayout& GetVertexLayout() const { return vertexBuffer->GetVertexLayout(); }

private:
  boost::shared_ptr<VertexBuffer> vertexBuffer;
  boost::shared_ptr<IndexBuffer> indexBuffer;
  GLuint vao;
};

#endif // __VERTEX_ARRAY__
