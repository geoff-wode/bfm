#include <core/logging.h>
#include <core/buffers/vertexbuffer.h>
#include <memory>

VertexBuffer::VertexBuffer(const VertexLayout& vertexLayout, size_t vertexCount, GLenum usage)
  : vertexCount(vertexCount),
    vertexLayout(vertexLayout)
{
  glGenBuffers(1, &buffer);
  Enable();
  glBufferData(GL_ARRAY_BUFFER, vertexLayout.GetStride() * vertexCount, NULL, usage);
  Disable();
}

VertexBuffer::~VertexBuffer()
{
  glDeleteBuffers(1, &buffer);
}

void VertexBuffer::SetData(const void* const data, size_t vertexCount, size_t startVertex)
{
  const size_t stride = vertexLayout.GetStride();
  glBufferSubData(GL_ARRAY_BUFFER, stride * startVertex, stride * vertexCount, data);
}
