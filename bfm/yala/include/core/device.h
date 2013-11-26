#if ! defined(__DEVICE__)
#define __DEVICE__

#include <glm/glm.hpp>

#include <core/effect/effect.h>
#include <core/vertexarray.h>
#include <core/buffers/indexbuffer.h>
#include <core/buffers/vertexbuffer.h>
#include <core/window.h>

//----------------------------------------------------------

// Creates resources that may be shared between contexts.
namespace Device
{
  WindowPtr NewWindow(
    const char* const title,
    const glm::ivec2& size,
    bool fullScreen,
    bool visible);

  WindowPtr NewWindow(
    const char* const title,
    const glm::ivec2& position,
    const glm::ivec2& size,
    bool fullScreen,
    bool visible);

  VertexArrayPtr NewVertexArray(VertexBufferPtr vertexBuffer);
  VertexArrayPtr NewVertexArray(VertexBufferPtr vertexBuffer, IndexBufferPtr indexBuffer);

  VertexBufferPtr NewVertexBuffer(const VertexLayout& layout, size_t vertexCount, GLenum usage);

  IndexBufferPtr NewIndexBuffer(size_t indexCount, GLenum indexType, GLenum usage);
};

#endif // __DEVICE__
 