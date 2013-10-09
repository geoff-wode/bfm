#include <vertexarray.h>
#include <boost/foreach.hpp>

static GLuint Initialise(boost::shared_ptr<VertexBuffer> vertexBuffer, boost::shared_ptr<IndexBuffer> indexBuffer)
{
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  vertexBuffer->Enable();
  if (indexBuffer) { indexBuffer->Enable(); }

  BOOST_FOREACH(auto attr, vertexBuffer->GetVertexLayout().GetAttributes())
  {
    glEnableVertexAttribArray(attr.semantic);
    glVertexAttribPointer(
      attr.semantic,
      attr.elements,
      attr.type,
      GL_FALSE,
      vertexBuffer->GetVertexLayout().GetStride(),
      (const void*)attr.offset);
  }

  glBindVertexArray(0);
  vertexBuffer->Disable();
  if (indexBuffer) { indexBuffer->Disable(); }

  return vao;
}

VertexArray::VertexArray(boost::shared_ptr<VertexBuffer> vertexBuffer)
  : vertexBuffer(vertexBuffer),
    vao(Initialise(vertexBuffer, NULL))
{
}

VertexArray::VertexArray(boost::shared_ptr<VertexBuffer> vertexBuffer, boost::shared_ptr<IndexBuffer> indexBuffer)
  : vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
    vao(Initialise(vertexBuffer, indexBuffer))
{
}

VertexArray::~VertexArray()
{
  glDeleteVertexArrays(1, &vao);
}
