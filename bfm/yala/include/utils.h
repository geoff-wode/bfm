#if ! defined(__UTILS__)
#define __UTILS__

#include <glm/glm.hpp>

// Encode a double precision value in 2 single-precision ones.
// See http://blogs.agi.com/insight3d/index.php/2008/09/03/precisions-precisions
// for the unexpurgated horror.
void DoubleToFloat(double value, float& low, float& high);

void DoubleToFloat(const glm::dvec2& value, glm::vec2& low, glm::vec2& high);
void DoubleToFloat(const glm::dvec3& value, glm::vec3& low, glm::vec3& high);
void DoubleToFloat(const glm::dvec4& value, glm::vec4& low, glm::vec4& high);

#endif // __UTILS__
