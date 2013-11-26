#if ! defined(__UTILS__)
#define __UTILS__

#include <glm/glm.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

// Allow for arbitrary functions (with parameters) to execute on block exit.
// Multiple "OnExit" functions will be invoked in reverse order of declaration.
#define CONCAT(x, y) x ## y
#define AUTO_VAR(x, y) CONCAT(x, y)
#define UNIQUE_VAR(x) AUTO_VAR(x, __COUNTER__)
#define CALLONEXIT(...) const boost::shared_ptr<void> UNIQUE_VAR(onexit_) (NULL, boost::bind<void>(__VA_ARGS__))

// Encode a double precision value in 2 single-precision ones.
// See http://blogs.agi.com/insight3d/index.php/2008/09/03/precisions-precisions
// for the unexpurgated horror.
void DoubleToFloat(double value, float& low, float& high);

void DoubleToFloat(const glm::dvec2& value, glm::vec2& low, glm::vec2& high);
void DoubleToFloat(const glm::dvec3& value, glm::vec3& low, glm::vec3& high);
void DoubleToFloat(const glm::dvec4& value, glm::vec4& low, glm::vec4& high);

#endif // __UTILS__
