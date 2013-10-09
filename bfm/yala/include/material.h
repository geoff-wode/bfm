#if ! defined(__MATERIAL__)
#define __MATERIAL__

#include <glm/glm.hpp>
#include <boost/shared_ptr.hpp>
#include <texture2d.h>

struct Material
{
  glm::vec3 diffuseColour;  // over-ridden if NULL != diffuseTexture;
  boost::shared_ptr<Texture2D> diffuseTexture;
};

#endif // __MATERIAL__
