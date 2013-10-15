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
  VertexArray();
  ~VertexArray();

  void Initialise(boost::shared_ptr<VertexBuffer> vertexBuffer);
  void Initialise(boost::shared_ptr<VertexBuffer> vertexBuffer, boost::shared_ptr<IndexBuffer> indexBuffer);

  void Enable() { glBindVertexArray(vao); }

  boost::shared_ptr<VertexBuffer> GetVertexBuffer() const { return vertexBuffer; }
  boost::shared_ptr<IndexBuffer> GetIndexBuffer() const { return indexBuffer; }

  const VertexLayout& GetVertexLayout() const { return vertexBuffer->GetVertexLayout(); }

private:
  GLuint vao;
  boost::shared_ptr<VertexBuffer> vertexBuffer;
  boost::shared_ptr<IndexBuffer> indexBuffer;
};

#endif // __VERTEX_ARRAY__
