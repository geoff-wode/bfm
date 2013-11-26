#if ! defined(__INDEX_BUFFER__)
#define __INDEX_BUFFER__

#include <cstddef>
#include <gl_loader/gl_loader.h>
#include <boost/shared_ptr.hpp>

class IndexBuffer
{
public:
  IndexBuffer(size_t indexCount, GLenum indexType, GLenum usage);
  ~IndexBuffer();

  void Enable() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); }
  static void Disable() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

  void SetData(const void* const data, size_t indexCount, size_t startIndex = 0);

  GLenum  GetIndexType() const { return indexType; }
  size_t GetIndexCount() const { return indexCount; }

private:
  const GLenum indexType;
  const size_t indexCount;
  const size_t typeSize;
  GLuint buffer;
};

typedef boost::shared_ptr<IndexBuffer> IndexBufferPtr;

#endif // __INDEX_BUFFER__
