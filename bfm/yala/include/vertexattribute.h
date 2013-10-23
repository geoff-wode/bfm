#if ! defined(__VERTEX_ATTRIBUTE__)
#define __VERTEX_ATTRIBUTE__

#include <cstddef>
#include <gl_loader/gl_loader.h>

namespace VertexSemantic
{
  enum Enum
  {
    Position,
    Normal,
    Texture0,
    Texture1,
    Texture2,
    Texture3,
    Texture4,
    Texture5,
    Texture6,
    Texture7,
    Texture8,
    Texture9,
    Texture10,
    Texture11,
    Texture12,
    Texture13,
    Texture14,
    Texture15
  };
}

struct VertexAttribute
{
  VertexSemantic::Enum semantic;
  GLenum type;                    // float, uint, etc.
  size_t elements;                // 1 for float, 2 for vec2, etc.
  size_t offset;                  // byte offset from start of vertex structure
};

#endif // __VERTEX_ATTRIBUTE__
