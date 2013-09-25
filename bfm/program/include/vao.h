#if ! defined(__VAO__)
#define __VAO__

#include <boost/shared_ptr.hpp>
#include <glbuffers.h>
#include <gl_loader/gl_loader.h>

struct VAO
{
  GLuint handle;
  size_t indexCount;

  boost::shared_ptr<VertexBuffer> vertexBuffer;
  boost::shared_ptr<IndexBuffer> indexBuffer;

  VAO() { glGenVertexArrays(1, &handle); }
  ~VAO() { glDeleteVertexArrays(1, &handle); }

  void Bind() { glBindVertexArray(handle); }
  static void Unbind() { glBindVertexArray(0); }
};

#endif // __VAO__
