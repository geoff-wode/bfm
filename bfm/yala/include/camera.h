#if ! defined(__CAMERA__)
#define __CAMERA__

#include <glm/glm.hpp>

class Camera
{
public:
  Camera();

  void Initialise(size_t screenWidth, size_t screenHeight);
  void Update(double elapsedMS);

  double fov;
  double tanFoVOver2;          // == tan(field_of_view_angle / 2)
  size_t displayResolutionX;  // number of pixels in X
  double aspectRatio;
  double nearClip;
  double farClip;
  double yaw;
  double pitch;
  double roll;
  double speed;
  glm::dvec3 movement;
  glm::dvec3 position;
  glm::dvec3 up;
  glm::dvec3 forward;
  glm::dvec3 right;
  glm::dmat4 viewMatrix;
  glm::dmat4 projectionMatrix;
  glm::dmat4 viewProjectionMatrix;
};

#endif // __CAMERA__
