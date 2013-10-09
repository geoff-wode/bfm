#include <uniformbuffer.h>


UniformBuffer::UniformBuffer(size_t size)
  : size(size)
{
  glGenBuffers(1, &buffer);
  Enable();
  glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STREAM_DRAW);
  Disable();
}

UniformBuffer::~UniformBuffer()
{
  glDeleteBuffers(1, &buffer);
}

void UniformBuffer::SetData(const void* const data, size_t size, size_t offset)
{
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}


void UniformBuffer::BindTo(GLuint bindingPoint)
{
  BindTo(bindingPoint, size, 0);
}

void UniformBuffer::BindTo(GLuint bindingPoint, size_t size, size_t offset)
{
  glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, buffer, offset, size);
}
