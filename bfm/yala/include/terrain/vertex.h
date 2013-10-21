#if ! defined(__VERTEX__)
#define __VERTEX__

#include <glm/glm.hpp>
#include <utils.h>

// A vertex position is double-precision, encoded into 2 single-precision values.
struct Vertex
{
  glm::vec3 positionLow;
  glm::vec3 positionHigh;

  Vertex& operator=(const glm::dvec3& value)
  {
    DoubleToFloat(value, positionLow, positionHigh);
    return *this;
  }
};

#endif // __VERTEX__
