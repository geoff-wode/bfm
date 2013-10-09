#if ! defined(__MESH__)
#define __MESH__

#include <material.h>
#include <vertexarray.h>
#include <boost/shared_ptr.hpp>

struct Mesh
{
  boost::shared_ptr<VertexArray>  vertexArray;
  boost::shared_ptr<Material>     material;
};

#endif // __MESH__
