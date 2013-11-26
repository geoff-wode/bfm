#if ! defined(__CAMERA__)
#define __CAMERA__

#include <glm/glm.hpp>

class Camera
{
public:
  Camera();
  virtual ~Camera();

  virtual void Update(float elapsedMS);

  double nearClip;
  double farClip;
  double aspectRatio;
  double fieldOfViewAngle;

  glm::dvec3 position;
  glm::dvec3 target;
  glm::dvec3 forward;
  glm::dvec3 right;
  glm::dvec3 up;
  glm::dmat4 viewMatrix;
  glm::dmat4 projectionMatrix;
};

#endif // __CAMERA__
