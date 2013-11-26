#include <core/vertexarray.h>
#include <boost/foreach.hpp>

//------------------------------------------------------------------------

VertexArray::~VertexArray()
{
  glDeleteVertexArrays(1, &vao);
}

//------------------------------------------------------------------------

VertexArray::VertexArray(boost::shared_ptr<VertexBuffer> vertexBuffer)
{
  Initialise(vertexBuffer, NULL);
}

//------------------------------------------------------------------------

VertexArray::VertexArray(boost::shared_ptr<VertexBuffer> vertexBuffer, boost::shared_ptr<IndexBuffer> indexBuffer)
{
  Initialise(vertexBuffer, indexBuffer);
}

//------------------------------------------------------------------------

void VertexArray::Initialise(boost::shared_ptr<VertexBuffer> vertexBuffer, boost::shared_ptr<IndexBuffer> indexBuffer)
{
  this->vertexBuffer = vertexBuffer;
  this->indexBuffer = indexBuffer;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  vertexBuffer->Enable();
  if (indexBuffer)
  {
    indexBuffer->Enable();
  }

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
  if (indexBuffer)
  {
    indexBuffer->Disable();
  }
}
