#if ! defined(__GLBUFFERS__)
#define __GLBUFFERS__

#include <cstddef>
#include <gl_loader/gl_loader.h>

struct GLBuffer
{
  GLuint handle;

  GLBuffer() { glGenBuffers(1, &handle); }
  virtual ~GLBuffer() { glDeleteBuffers(1, &handle); }
};

struct VertexBuffer : public GLBuffer
{
  const size_t vertexSize;

  VertexBuffer(size_t vertexSize, size_t vertexCount, GLenum usage)
    : vertexSize(vertexSize)
  {
    Initialise(vertexCount, usage, NULL);
  }

  VertexBuffer(size_t vertexSize, size_t vertexCount, GLenum usage, const void* const data)
    : vertexSize(vertexSize)
  {
    Initialise(vertexCount, usage, data);
  }

  void SetData(size_t vertexCount, size_t startVertex, const void* const data)
  {
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize * startVertex, vertexSize * vertexCount, data);
    Unbind();
  }

  void Bind() { glBindBuffer(GL_ARRAY_BUFFER, handle); }
  static void Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

private:
  void Initialise(size_t vertexCount, GLenum usage, const void* const data)
  {
    Bind();
    glBufferData(GL_ARRAY_BUFFER, vertexSize * vertexCount, data, usage);
    Unbind();
  }
};

struct IndexBuffer : public GLBuffer
{
  const GLenum type;
  const size_t typeSize;

  IndexBuffer(GLenum type, size_t typeSize, size_t indexCount, GLenum usage)
    : type(type), typeSize(typeSize)
  {
    Initialise(indexCount, usage, NULL);
  }

  IndexBuffer(GLenum type, size_t typeSize, size_t indexCount, GLenum usage, const void* const data)
    : type(type), typeSize(typeSize)
  {
    Initialise(indexCount, usage, data);
  }

  void SetData(size_t indexCount, size_t startIndex, const void* const data)
  {
    Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, typeSize * startIndex, typeSize * indexCount, data);
    Unbind();
  }

  void Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle); }
  static void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

private:
  void Initialise(size_t indexCount, GLenum usage, const void* const data)
  {
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, typeSize * indexCount, data, usage);
    Unbind();
  }
};

#endif // __GLBUFFERS__
