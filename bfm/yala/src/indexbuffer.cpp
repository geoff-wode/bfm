#include <indexbuffer.h>
#include <cstddef>

static size_t SizeOf(GLenum indexType)
{
  switch (indexType)
  {
  case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
  case GL_UNSIGNED_SHORT: return sizeof(GLushort);
  case GL_UNSIGNED_INT: return sizeof(GLuint);
  }
  return 0;
}


IndexBuffer::IndexBuffer(size_t indexCount, GLenum indexType, GLenum usage, const void* const data)
  : indexCount(indexCount),
    indexType(indexType),
    typeSize(SizeOf(indexType))
{
  glGenBuffers(1, &buffer);
  Enable();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, typeSize * indexCount, data, usage);
  Disable();
}

IndexBuffer::~IndexBuffer()
{
  glDeleteBuffers(1, &buffer);
}

void IndexBuffer::SetData(const void* const data, size_t indexCount, size_t startIndex)
{
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, typeSize * startIndex, typeSize * indexCount, data);
}
