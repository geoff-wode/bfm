#if ! defined(__UTILS__)
#define __UTILS__

#include <glm/glm.hpp>

// Encode a double precision in 2 floats.
// See http://blogs.agi.com/insight3d/index.php/2008/09/03/precisions-precisions
// for the unexpurgated horror.
void DoubleToFloat(double value, float& low, float& high);
void DoubleToFloat(const glm::dvec3& value, glm::vec3& low, glm::vec3& high);

#endif // __UTILS__
