#include <utils.h>

//-----------------------------------------------------------------
// Encode a double precision in 2 floats.
// See http://blogs.agi.com/insight3d/index.php/2008/09/03/precisions-precisions
// for the unexpurgated horror.
void DoubleToFloat(double value, float& low, float& high)
{
  static const double encoder = 1 << 16;

  if (value >= 0.0)
  {
    const double doubleHigh = floor(value / encoder) * encoder;
    high = (float)doubleHigh;
    low = (float)(value - doubleHigh);
  }
  else
  {
    const double doubleHigh = floor(-value / encoder) * encoder;
    high = (float)-doubleHigh;
    low = (float)(value + doubleHigh);
  }
}

//-----------------------------------------------------------------

void DoubleToFloat(const glm::dvec2& value, glm::vec2& low, glm::vec2& high)
{
  DoubleToFloat(value.x, low.x, high.x);
  DoubleToFloat(value.y, low.y, high.y);
}

//-----------------------------------------------------------------
void DoubleToFloat(const glm::dvec3& value, glm::vec3& low, glm::vec3& high)
{
  DoubleToFloat(value.x, low.x, high.x);
  DoubleToFloat(value.y, low.y, high.y);
  DoubleToFloat(value.z, low.z, high.z);
}

//-----------------------------------------------------------------
void DoubleToFloat(const glm::dvec4& value, glm::vec4& low, glm::vec4& high)
{
  DoubleToFloat(value.x, low.x, high.x);
  DoubleToFloat(value.y, low.y, high.y);
  DoubleToFloat(value.z, low.z, high.z);
  DoubleToFloat(value.w, low.w, high.w);
}
