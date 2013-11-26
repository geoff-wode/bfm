#include <boost/make_shared.hpp>
#include <core/device.h>

//------------------------------------------------------------------------

WindowPtr Device::NewWindow(
  const char* const title,
  const glm::ivec2& size,
  bool fullScreen,
  bool visible)
{
  WindowPtr window(new Window(title, size, fullScreen, visible));
  return window;
}

//------------------------------------------------------------------------

WindowPtr Device::NewWindow(
    const char* const title,
    const glm::ivec2& position,
    const glm::ivec2& size,
    bool fullScreen,
    bool visible)
{
  WindowPtr window(new Window(title, position, size, fullScreen, visible));
  return window;
}

//------------------------------------------------------------------------

VertexArrayPtr Device::NewVertexArray(VertexBufferPtr vertexBuffer)
{
  VertexArrayPtr va(new VertexArray(vertexBuffer));
  return va;
}

VertexArrayPtr Device::NewVertexArray(VertexBufferPtr vertexBuffer, IndexBufferPtr indexBuffer)
{
  VertexArrayPtr va(new VertexArray(vertexBuffer, indexBuffer));
  return va;
}

//------------------------------------------------------------------------

VertexBufferPtr Device::NewVertexBuffer(const VertexLayout& layout, size_t vertexCount, GLenum usage)
{
  VertexBufferPtr vb(new VertexBuffer(layout, vertexCount, usage));
  return vb;
}

//------------------------------------------------------------------------

IndexBufferPtr Device::NewIndexBuffer(size_t indexCount, GLenum indexType, GLenum usage)
{
  IndexBufferPtr ib(new IndexBuffer(indexCount, indexType, usage));
  return ib;
}
