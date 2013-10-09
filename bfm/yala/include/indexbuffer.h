#if ! defined(__INDEX_BUFFER__)
#define __INDEX_BUFFER__

#include <cstddef>
#include <gl_loader/gl_loader.h>

class IndexBuffer
{
public:
  IndexBuffer(size_t indexCount, GLenum indexType, GLenum usage, const void* const data = NULL);
  ~IndexBuffer();

  void Enable() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); }
  void Disable() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

  void SetData(const void* const data, size_t indexCount, size_t startIndex = 0);

  GLenum  GetIndexType() const { return indexType; }
  size_t GetIndexCount() const { return indexCount; }

private:
  GLenum indexType;
  size_t indexCount;
  size_t typeSize;
  GLuint buffer;
};

#endif // __INDEX_BUFFER__
