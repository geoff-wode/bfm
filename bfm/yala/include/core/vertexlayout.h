#if ! defined(__VERTEX_LAYOUT__)
#define __VERTEX_LAYOUT__

#include <vector>
#include <cstddef>
#include <gl_loader/gl_loader.h>
#include <core/vertexattribute.h>

class VertexLayout
{
public:
  VertexLayout();
  ~VertexLayout();

  void AddAttribute(const VertexAttribute& attr);

  size_t GetStride() const { return stride; }
  const std::vector<VertexAttribute>& GetAttributes() const { return attributes; }

private:
  size_t stride;
  std::vector<VertexAttribute> attributes;
};

#endif // __VERTEX_LAYOUT__
