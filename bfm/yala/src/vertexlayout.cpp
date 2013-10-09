#include <vertexlayout.h>

VertexLayout::VertexLayout()
  : stride(0)
{
}

VertexLayout::~VertexLayout()
{
}

void VertexLayout::AddAttribute(const VertexAttribute& attr)
{
  attributes.push_back(attr);

  switch (attr.type)
  {
  case GL_INT: stride += sizeof(int) * attr.elements; break;
  case GL_FLOAT: stride += sizeof(float) * attr.elements; break;
  case GL_UNSIGNED_INT: stride += sizeof(unsigned int) * attr.elements; break;
  default: break;
  }
}
