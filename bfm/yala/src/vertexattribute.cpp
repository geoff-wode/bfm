//#include <vertexattribute.h>
//
//GLenum VertexAttribute::GetElementType(GLenum dataType)
//{
//  switch (dataType)
//  {
//  case GL_FLOAT_VEC2: return GL_FLOAT;
//  case GL_FLOAT_VEC3: return GL_FLOAT;
//  case GL_FLOAT_VEC4: return GL_FLOAT;
//  case GL_INT_VEC2:   return GL_INT;
//  case GL_INT_VEC3:   return GL_INT;
//  case GL_INT_VEC4:   return GL_INT;
//  case GL_SAMPLER_1D: return GL_UNSIGNED_INT;
//  case GL_SAMPLER_2D: return GL_UNSIGNED_INT;
//  case GL_SAMPLER_3D: return GL_UNSIGNED_INT;
//  default: break;
//  }
//
//  return dataType;
//}
//
//size_t VertexAttribute::GetElementCount(GLenum dataType)
//{
//  switch (dataType)
//  {
//  case GL_FLOAT:      return 1;
//  case GL_FLOAT_VEC2: return 2;
//  case GL_FLOAT_VEC3: return 3;
//  case GL_FLOAT_VEC4: return 4;
//  case GL_INT:        return 1;
//  case GL_INT_VEC2:   return 2;
//  case GL_INT_VEC3:   return 3;
//  case GL_INT_VEC4:   return 4;
//  case GL_SAMPLER_1D: return 1;
//  case GL_SAMPLER_2D: return 1;
//  case GL_SAMPLER_3D: return 1;
//  default: break;
//  }
//
//  return 0;
//}
